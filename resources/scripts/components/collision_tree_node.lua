ffi.cdef[[
typedef enum collision_geom_type_e
{
  COLLISION_GEOM_TYPE_BOX = 0,
  COLLISION_GEOM_TYPE_SPHERE,
  COLLISION_GEOM_TYPE_CAPSULE
} CollisionGeomType;

typedef struct collision_tree_node_t
{
  CollisionGeomType type;
  UUID collisionVolume;
  UUID nextCollider;
  bool isTrigger;
  dGeomID geomID;
} CollisionTreeNode;
]]

local component = engine.components:register("collision_tree_node", "CollisionTreeNode")
