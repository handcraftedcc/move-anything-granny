#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <vector>

#include "granny_engine.h"

static void fail(const char *msg) {
    std::fprintf(stderr, "FAIL: %s\n", msg);
    std::exit(1);
}

static void require_true(bool cond, const char *msg) {
    if (!cond) fail(msg);
}

static void render_and_collect_spawns(grn_engine_t *engine,
                                      const float *sample,
                                      int sample_len,
                                      bool prev_active[GRN_MAX_VOICES][GRN_MAX_GRAINS_PER_VOICE_HIGH],
                                      std::vector<float> &spawn_gain_mag) {
    float left[GRN_MAX_RENDER];
    float right[GRN_MAX_RENDER];
    grn_engine_render(engine, sample, sample_len, GRN_SAMPLE_RATE, left, right, 128);

    for (int v = 0; v < GRN_MAX_VOICES; v++) {
        for (int g = 0; g < GRN_MAX_GRAINS_PER_VOICE_HIGH; g++) {
            bool is_active = engine->grains[v][g].active != 0;
            if (is_active && !prev_active[v][g]) {
                float gl = engine->grains[v][g].gain_l;
                float gr = engine->grains[v][g].gain_r;
                spawn_gain_mag.push_back(std::sqrt(gl * gl + gr * gr));
            }
            prev_active[v][g] = is_active;
        }
    }
}

static float avg_slice(const std::vector<float> &vals, int begin, int end) {
    if (end <= begin) return 0.0f;
    float sum = 0.0f;
    for (int i = begin; i < end; i++) sum += vals[(size_t)i];
    return sum / (float)(end - begin);
}

static void test_attack_ramps_spawn_gain() {
    grn_engine_t engine;
    grn_engine_init(&engine);

    grn_params_t p = engine.params;
    p.play_mode = GRN_PLAY_MODE_MONO;
    p.trigger_mode = GRN_TRIGGER_PER_VOICE;
    p.quality = GRN_QUALITY_HIGH;
    p.size_ms = 5.0f;
    p.density = 60.0f;
    p.spray = 0.0f;
    p.jitter = 0.0f;
    p.amp_attack_ms = 150.0f;
    p.amp_decay_ms = 0.0f;
    p.amp_sustain = 1.0f;
    p.amp_release_ms = 200.0f;
    grn_engine_set_params(&engine, &p);

    float sample[4096];
    for (int i = 0; i < 4096; i++) sample[i] = ((i % 97) - 48) / 48.0f;

    bool prev_active[GRN_MAX_VOICES][GRN_MAX_GRAINS_PER_VOICE_HIGH] = {{false}};
    std::vector<float> spawned;

    grn_engine_note_on(&engine, 60, 1.0f);
    for (int i = 0; i < 140; i++) {
        render_and_collect_spawns(&engine, sample, 4096, prev_active, spawned);
    }

    require_true(spawned.size() >= 6, "expected enough spawned grains to evaluate attack ramp");
    int n = (int)spawned.size();
    int q = n / 4;
    if (q < 1) q = 1;
    float early = avg_slice(spawned, 0, q);
    float late = avg_slice(spawned, n - q, n);
    require_true(late > early * 1.6f, "spawn gain should ramp up during attack");
}

static void test_release_extends_emission() {
    grn_engine_t engine;
    grn_engine_init(&engine);

    grn_params_t p = engine.params;
    p.play_mode = GRN_PLAY_MODE_MONO;
    p.trigger_mode = GRN_TRIGGER_PER_VOICE;
    p.quality = GRN_QUALITY_HIGH;
    p.size_ms = 5.0f;
    p.density = 60.0f;
    p.spray = 0.0f;
    p.jitter = 0.0f;
    p.amp_attack_ms = 0.0f;
    p.amp_decay_ms = 0.0f;
    p.amp_sustain = 1.0f;
    p.amp_release_ms = 500.0f;
    grn_engine_set_params(&engine, &p);

    float sample[4096];
    for (int i = 0; i < 4096; i++) sample[i] = ((i % 61) - 30) / 30.0f;

    bool prev_active[GRN_MAX_VOICES][GRN_MAX_GRAINS_PER_VOICE_HIGH] = {{false}};
    std::vector<float> spawned;

    grn_engine_note_on(&engine, 60, 1.0f);
    for (int i = 0; i < 30; i++) {
        render_and_collect_spawns(&engine, sample, 4096, prev_active, spawned);
    }
    grn_engine_note_off(&engine, 60);

    std::vector<float> post_off_spawned;
    for (int i = 0; i < 90; i++) {
        render_and_collect_spawns(&engine, sample, 4096, prev_active, post_off_spawned);
    }

    require_true(!post_off_spawned.empty(), "release phase should continue spawning grains");
}

static void test_sustain_zero_stops_emission_while_held() {
    grn_engine_t engine;
    grn_engine_init(&engine);

    grn_params_t p = engine.params;
    p.play_mode = GRN_PLAY_MODE_MONO;
    p.trigger_mode = GRN_TRIGGER_PER_VOICE;
    p.quality = GRN_QUALITY_HIGH;
    p.size_ms = 5.0f;
    p.density = 60.0f;
    p.spray = 0.0f;
    p.jitter = 0.0f;
    p.amp_attack_ms = 0.0f;
    p.amp_decay_ms = 10.0f;
    p.amp_sustain = 0.0f;
    p.amp_release_ms = 500.0f;
    grn_engine_set_params(&engine, &p);

    float sample[4096];
    for (int i = 0; i < 4096; i++) sample[i] = ((i % 89) - 44) / 44.0f;

    bool prev_active[GRN_MAX_VOICES][GRN_MAX_GRAINS_PER_VOICE_HIGH] = {{false}};
    std::vector<float> spawned;

    grn_engine_note_on(&engine, 60, 1.0f);
    for (int i = 0; i < 120; i++) {
        render_and_collect_spawns(&engine, sample, 4096, prev_active, spawned);
    }

    std::vector<float> late_spawned;
    for (int i = 0; i < 80; i++) {
        render_and_collect_spawns(&engine, sample, 4096, prev_active, late_spawned);
    }

    require_true(late_spawned.empty(), "sustain=0 should stop new spawns after decay while note is held");
}

int main() {
    test_attack_ramps_spawn_gain();
    test_release_extends_emission();
    test_sustain_zero_stops_emission_while_held();
    std::printf("PASS: granny spawn ADSR behavior\n");
    return 0;
}
