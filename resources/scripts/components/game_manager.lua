ffi.cdef[[
typedef struct game_manager_component_t
{
    uint32 playerScore;
    uint32 opponentScore;
    UUID playerScoreText;
    UUID opponentScoreText;
    uint32 numBalls;
    UUID opponentPaddle;
} GameManagerComponent;
]]

local component = engine.components:register("game_manager", "GameManagerComponent")
