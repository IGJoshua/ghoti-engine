ffi.cdef[[
typedef struct joint_constraint_component_t
{
	bool loStop_bool;
	bool hiStop_bool;
	bool stopBouncyness_bool;
	bool CFM_bool;
	bool stopERP_bool;
	bool stopCFM_bool;
	real32 loStop_val;
	real32 hiStop_val;
	real32 stopBouncyness_val;
	real32 CFM_val;
	real32 stopERP_val;
	real32 stopCFM_val;
} JointConstraintComponent;
]]

local component = engine.components:register("joint_constraint", "JointConstraintComponent")