#pragma once

// Touch feed shared between InputConsumer hook / Unity backup / watermark hit-test.
namespace kik_input
{
    // Edge-triggered: true once when a tap begins inside [x0,y0]–[x1,y1].
    bool consume_tap_in_rect(float x0, float y0, float x1, float y1);
}
