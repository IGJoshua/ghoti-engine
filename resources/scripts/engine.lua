ffi = require("ffi")

engine = {}

engine.C = ffi.load(ffi.os == "Windows"
					  and "build/monochrome.dll"
					  or "build/monochrome.so")

require("resources/scripts/cdefs")

local Scene = require("resources/scripts/scene")

local C = engine.C

engine.scenes = {}

function engine.initScene(pScene)
  local scene = Scene.new(pScene)

  -- Find all physics systems in the scene
  local list = ffi.new("List[1]", scene.ptr.luaPhysicsFrameSystemNames)
  local itr = ffi.new(
	"ListIterator",
	C.listGetIterator(
	  list))

  io.write("Got physics iterator\n")

  local i = 1
  while C.listIteratorAtEnd(itr) == 0 do
	io.write(string.format("Adding physics system number: %d\n", i))

	io.write(ffi.string(ffi.cast("UUID *", itr[0].data).string))
	-- Load all the systems named
	local physicsSystem = require(string.format(
				"resources/scripts/systems/%s",
				ffi.string(ffi.cast("UUID *", itr[0].data).string)))

	-- Call all the init functions
	if physicsSystem.init then
	  physicsSystem.init(scene)
	end

	scene.physicsSystems[i] = physicsSystem

	local itrRef = ffi.new("ListIterator[1]", itr)
	C.listMoveIterator(itrRef)
	itr = itrRef[0]
	i = i + 1
  end

  scene.numPhysicsFrameSystems = i - 1

  io.write(string.format("Loaded %d physics systems\n", scene.numPhysicsFrameSystems))

  -- Find all render systems in the scene
  list = ffi.new("List[1]", scene.ptr.luaPhysicsFrameSystemNames)
  itr = ffi.new(
	"ListIterator",
	C.listGetIterator(
	  list))

  io.write("Got rendering iterator\n")

  i = 1
  while not C.listIteratorAtEnd(itr) do
	io.write(string.format("Adding render system number: %d\n", i))
	-- Load all the systems named
	local renderSystem = require(string.format(
								   "resources/scripts/systems/%s",
								   C.listGetIterator(itr)))

	-- Call all the init functions
	if renderSystem.init then
	  renderSystem.init(scene)
	end

	scene.renderSystems[i] = renderSystem

	local itrRef = ffi.new("ListIterator[1]", itr)
	C.listMoveIterator(itrRef)
	itr = itrRef[0]
	i = i + 1
  end

  scene.numRenderFrameSystems = i - 1

  io.write(string.format("Loaded %d render systems\n", scene.numPhysicsFrameSystems))

  engine.scenes[pScene] = scene
end

function engine.runPhysicsSystems(pScene, dt)
  local scene = engine.scenes[pScene]

  local null = ffi.new("int64", 0)
  local emptyUUID = ffi.new("UUID")

  for i = 1,scene.numPhysicsFrameSystems do
	local physicsSystem = scene.physicsSystems[i]

	if physicsSystem.run then
	  local componentName = ffi.new("UUID[1]", C.idFromName(physicsSystem.components[1]))
	  -- Loop over every entity which has the correct components
	  local firstComp = ffi.cast(
		"ComponentDataTable **",
		C.hashMapGetKey(
		  scene.ptr.componentTypes,
		  componentName))

	  if ffi.cast("int64", firstComp) ~= null and ffi.cast("int64", firstComp[0]) ~= null then
		for j = 0,tonumber(firstComp[0].numEntries) - 1 do
		  if ffi.string(emptyUUID.string) ~= ffi.string(
			ffi.cast("UUID *", firstComp[0].data + j
					   * (firstComp[0].componentSize
							+ ffi.sizeof("UUID"))).string) then

			local valid = true
			for k = 2,physicsSystem.numComponents do
			  local componentID = C.idFromName(physicsSystem.components[k])
			  local componentTable = ffi.cast(
				"ComponentDataTable **",
				C.hashMapGetKey(
				  scene.ptr.componentTypes,
				  componentID))

			  if ffi.cast("int64", componentTable) ~= null and ffi.cast("int64", componentTable[0]) then
				if ffi.cast("int64", ffi.cast("uint32 *", C.hashMapGetKey(
												componentTable[0].idToIndex,
												firstComp[0].data
												  + j
												  * (firstComp[0].componentSize
													 + ffi.sizeof("UUID"))))) == null then
				  valid = false
				  break
				end
			  end
			end

			if valid then
			  physicsSystem.run(scene,
								ffi.cast("UUID *", firstComp[0].data
										   + j
										   * (firstComp[0].componentSize
												+ ffi.sizeof("UUID"))),
								dt)
			end
		  end
		end
	  end
	end
  end
end

function engine.runRenderSystems(pScene, dt)
  local scene = engine.scenes[pScene]

  local null = ffi.new("int64", 0)
  local emptyUUID = ffi.new("UUID")

  for i = 1,scene.numRenderFrameSystems do
	local renderSystem = scene.renderSystems[i]

	if renderSystem.run then
	  local componentName = ffi.new("UUID[1]", C.idFromName(renderSystem.components[1]))
	  -- Loop over every entity which has the correct components
	  local firstComp = ffi.cast(
		"ComponentDataTable **",
		C.hashMapGetKey(
		  scene.ptr.componentTypes,
		  componentName))

	  if ffi.cast("int64", firstComp) ~= null and ffi.cast("int64", firstComp[0]) ~= null then
		for j = 0,tonumber(firstComp[0].numEntries) - 1 do
		  if ffi.string(emptyUUID.string) ~= ffi.string(
			ffi.cast("UUID *", firstComp[0].data + j
					   * (firstComp[0].componentSize
							+ ffi.sizeof("UUID"))).string) then

			local valid = true
			for k = 2,renderSystem.numComponents do
			  local componentID = C.idFromName(renderSystem.components[k])
			  local componentTable = ffi.cast(
				"ComponentDataTable **",
				C.hashMapGetKey(
				  scene.ptr.componentTypes,
				  componentID))

			  if ffi.cast("int64", componentTable) ~= null and ffi.cast("int64", componentTable[0]) then
				if ffi.cast("int64", ffi.cast("uint32 *", C.hashMapGetKey(
												componentTable[0].idToIndex,
												firstComp[0].data
												  + j
												  * (firstComp[0].componentSize
													 + ffi.sizeof("UUID"))))) == null then
				  valid = false
				  break
				end
			  end
			end

			if valid then
			  renderSystem.run(scene,
								ffi.cast("UUID *", firstComp[0].data
										   + j
										   * (firstComp[0].componentSize
												+ ffi.sizeof("UUID"))),
								dt)
			end
		  end
		end
	  end
	end
  end
end

function engine.shutdownScene(pScene)
  local scene = engine.scenes[pScene]

  io.write(string.format(
			 "Scene has %d physics systems\n",
			 scene.numPhysicsFrameSystems))

  -- TODO: Call all the shutdown methods on the systems
  for i = 1,scene.numPhysicsFrameSystems do
	if scene.physicsSystems[i].shutdown then
	  scene.physicsSystems[i].shutdown(scene)
	end
  end

  io.write(string.format(
			 "Scene has %d render systems\n",
			 scene.numRenderFrameSystems))

  for i = 1,scene.numRenderFrameSystems do
	if scene.renderSystems[i].shutdown then
	  scene.renderSystems[i].shutdown(scene)
	end
  end
end
