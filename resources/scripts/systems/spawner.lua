local system = {}

local kazmath = engine.kazmath
local C = engine.C

local vecOut = ffi.new("kmVec3[1]")
local quatOut = ffi.new("kmQuaternion[1]")

system.components = {}
system.components[1] = "spawner"
system.components[2] = "transform"

function system.run(scene, entityID, dt)
  local spawner = scene:getComponent("spawner", entityID)

  if spawner.numSpawned >= spawner.numToSpawn then
	return nil
  end

  local spawnerTransform = scene:getComponent("transform", entityID)

  spawner.timeElapsed = spawner.timeElapsed + dt

  local numToSpawn =
	math.floor(spawner.spawnPerSecond * spawner.timeElapsed + 0.5)
	- spawner.numSpawned

  for i=1,numToSpawn do
	spawner.numSpawned = spawner.numSpawned + 1

	if spawner.numSpawned > spawner.numToSpawn then
	  return nil
	end

	print(string.format(
			"Spawning entity #%d from spawner %s",
			spawner.numSpawned,
			ffi.string(entityID.string)))

	-- Create an entity with a box model
	local entity = C.sceneCreateEntity(scene.ptr)
	local transform = ffi.new("TransformComponent")
	transform.dirty = true

	kazmath.kmVec3Assign(vecOut, spawnerTransform.globalPosition)
	transform.position = vecOut[0]
	transform.globalPosition = vecOut[0]
	transform.lastGlobalPosition = vecOut[0]

	kazmath.kmQuaternionIdentity(quatOut)
	transform.rotation = quatOut[0]
	transform.globalRotation = quatOut[0]
	transform.lastGlobalRotation = quatOut[0]

	kazmath.kmVec3Fill(vecOut, 1, 1, 1)
	transform.scale = vecOut[0]
	transform.globalScale = vecOut[0]
	transform.lastGlobalScale = vecOut[0]

	scene:addComponentToEntity("transform", entity, transform)

	local model = ffi.new("ModelComponent")
	model.name = "box"
	model.visible = true
	scene:addComponentToEntity("model", entity, model)

	local rigidbody = ffi.new("RigidBodyComponent")
	rigidbody.enabled = true
	if math.random(0, 1) >= 0.5 then
	  rigidbody.dynamic = true
	else
	  rigidbody.dynamic = false
	end
	rigidbody.gravity = true
	rigidbody.mass = 1
	C.kmVec3Zero(rigidbody.centerOfMass)
	rigidbody.velocity.x = math.random(-10, 10)
	rigidbody.velocity.y = math.random(-10, 10)
	rigidbody.velocity.z = math.random(-10, 10)
	C.kmVec3Zero(rigidbody.angularVel)
	rigidbody.defaultDamping = true
	rigidbody.maxAngularSpeed = 1000000
	rigidbody.inertiaType = 2
	rigidbody.moiParams[0] = 1
	rigidbody.moiParams[1] = 1
	rigidbody.moiParams[2] = 1

	scene:addComponentToEntity("rigid_body", entity, rigidbody)

	local collision = ffi.new("CollisionComponent")
	collision.hitList = C.idFromName("")
	collision.lastHitList = C.idFromName("")

	local colliderEntity = C.sceneCreateEntity(scene.ptr)

	local colliderTransform = ffi.new("TransformComponent")
	kazmath.kmVec3Assign(vecOut, spawnerTransform.globalPosition)
	colliderTransform.globalPosition = vecOut[0]
	colliderTransform.lastGlobalPosition = vecOut[0]
	kazmath.kmVec3Zero(vecOut)
	colliderTransform.position = vecOut[0]

	kazmath.kmQuaternionIdentity(quatOut)
	colliderTransform.rotation = quatOut[0]
	colliderTransform.globalRotation = quatOut[0]
	colliderTransform.lastGlobalRotation = quatOut[0]

	kazmath.kmVec3Fill(vecOut, 1, 1, 1)
	colliderTransform.scale = vecOut[0]
	colliderTransform.globalScale = vecOut[0]
	colliderTransform.lastGlobalScale = vecOut[0]

	colliderTransform.dirty = true

	transform.firstChild = colliderEntity
	colliderTransform.parent = entity
	scene:addComponentToEntity("transform", colliderEntity, colliderTransform)

	local collisionTreeNode = ffi.new("CollisionTreeNode")
	collisionTreeNode.type = 0
	collisionTreeNode.collisionVolume = entity
	collisionTreeNode.nextCollider = C.idFromName("")
	collisionTreeNode.isTrigger = false
	scene:addComponentToEntity("collision_tree_node", colliderEntity, collisionTreeNode)

	local box = ffi.new("BoxComponent")
	box.bounds.x = 1
	box.bounds.y = 1
	box.bounds.z = 1
	scene:addComponentToEntity("box", colliderEntity, box)

	collision.collisionTree = colliderEntity

	scene:addComponentToEntity("collision", entity, collision)

	C.registerRigidBody(scene.ptr, entity)
  end
end

return system
