ffi.cdef[[
typedef struct collision_component_t
{
  UUID collisionTree;
  UUID hitList;
  UUID lastHitList;
} CollisionComponent;
]]

local component = engine.components:register("collision", "CollisionComponent")
