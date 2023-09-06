#pragma once

// Null -> Disconnected -> Connecting -> Connected -> [Idle -> Busy]* .... -> Disconnecting -> Disconnected
enum class CameraState {
    Null,           // Initial State
    Disconnected,   // Constructor/Disconnect would set this
    Connecting,     // Connect method would set this
    Connected,      // Connect method would set this
    Disconnecting,  // Disconnect method would set this
};