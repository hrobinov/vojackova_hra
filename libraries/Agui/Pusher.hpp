#pragma once

// Signal accepted by Horizontal/Vertical Flow that adds Horizontally/Vertically stretchable item to it to push other elements
namespace agui { class Pusher{}; static constexpr Pusher pusher; }
namespace agui { class HorizontalPusher{}; static constexpr HorizontalPusher hPusher; }
namespace agui { class VerticalPusher{}; static constexpr VerticalPusher vPusher; }
