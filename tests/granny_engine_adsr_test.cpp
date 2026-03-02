#include <cstdio>
#include <cstdlib>

#include "granny_engine.h"

static void fail(const char *msg) {
    std::fprintf(stderr, "FAIL: %s\n", msg);
    std::exit(1);
}

static void require_true(bool cond, const char *msg) {
    if (!cond) fail(msg);
}

static void render_blocks(grn_engine_t *engine, const float *sample, int sample_len, int blocks) {
    float left[GRN_MAX_RENDER];
    float right[GRN_MAX_RENDER];
    for (int i = 0; i < blocks; i++) {
        grn_engine_render(engine, sample, sample_len, GRN_SAMPLE_RATE, left, right, 128);
    }
}

static void test_release_keeps_emitting() {
    grn_engine_t engine;
    grn_engine_init(&engine);

    grn_params_t p = engine.params;
    p.play_mode = GRN_PLAY_MODE_MONO;
    p.trigger_mode = GRN_TRIGGER_PER_VOICE;
    p.size_ms = 5.0f;
    p.density = 60.0f;
    p.amp_attack_ms = 0.0f;
    p.amp_decay_ms = 0.0f;
    p.amp_sustain = 1.0f;
    p.amp_release_ms = 600.0f;
    grn_engine_set_params(&engine, &p);

    float sample[2048];
    for (int i = 0; i < 2048; i++) sample[i] = ((i % 97) - 48) / 48.0f;

    grn_engine_note_on(&engine, 60, 1.0f);
    render_blocks(&engine, sample, 2048, 10);
    grn_engine_note_off(&engine, 60);

    bool saw_grains_late_in_release = false;
    for (int b = 0; b < 80; b++) {
        render_blocks(&engine, sample, 2048, 1);
        if (b > 10 && grn_engine_active_grains(&engine) > 0) {
            saw_grains_late_in_release = true;
            break;
        }
    }

    require_true(saw_grains_late_in_release, "release should continue grain emission");
}

static void test_decay_to_zero_stops_emission_early() {
    grn_engine_t engine;
    grn_engine_init(&engine);

    grn_params_t p = engine.params;
    p.play_mode = GRN_PLAY_MODE_MONO;
    p.trigger_mode = GRN_TRIGGER_PER_VOICE;
    p.size_ms = 5.0f;
    p.density = 60.0f;
    p.amp_attack_ms = 0.0f;
    p.amp_decay_ms = 8.0f;
    p.amp_sustain = 0.0f;
    p.amp_release_ms = 300.0f;
    grn_engine_set_params(&engine, &p);

    float sample[2048];
    for (int i = 0; i < 2048; i++) sample[i] = ((i % 61) - 30) / 30.0f;

    grn_engine_note_on(&engine, 60, 1.0f);
    render_blocks(&engine, sample, 2048, 260);

    require_true(engine.voices[0].gate == 1, "test expects held note");

    int max_grains_after_decay = 0;
    for (int b = 0; b < 80; b++) {
        render_blocks(&engine, sample, 2048, 1);
        int active = grn_engine_active_grains(&engine);
        if (active > max_grains_after_decay) {
            max_grains_after_decay = active;
        }
    }

    require_true(max_grains_after_decay == 0, "sustain=0 should stop new emission after decay");
}

int main() {
    test_release_keeps_emitting();
    test_decay_to_zero_stops_emission_early();
    std::printf("PASS: granny ADSR behavior\n");
    return 0;
}
