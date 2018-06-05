ffi = require("ffi")

engine = {}

-- io.output("log.txt")

io.write("Loaded cFFI\n")

engine.C = ffi.load(ffi.os == "Windows"
					  and "./monochrome.dll"
					  or "./monochrome.so")

local C = engine.C

io.write("Loaded monochrome library\n")

engine.kazmath = ffi.load(ffi.os == "Windows"
							and "winlib/libkazmath.a"
							or "lualib/libkazmath.so")

io.write("Loaded kazmath library\n")

require("resources/scripts/cdefs")

io.write("Created cffi definitions\n")

local Scene = require("resources/scripts/scene")

io.write("Required scene\n")

engine.components = require("resources/scripts/components")

-- TODO: iterate over every file in resources/scripts/components/ and require them
local testFile = io.popen(ffi.os == "Windows" and 'dir /b resources\\scripts\\components' or 'find resources/scripts/components -name "*.lua"')
for line in testFile:lines() do
  if ffi.os == "Windows" then
	line = 'resources/scripts/components/'..line
  end
  require(string.sub(line, 0, -5))
end

engine.scenes = {}

function engine.initScene(pScene)
  local scene = Scene:new(pScene)

  -- TODO: Register all lua components into the scene
  for name, component in pairs(engine.components) do
	-- If the component does not exist in the scene
	if type(component) == 'table' then
	  if C.hashMapGetKey(scene.ptr.componentTypes, C.idFromName(name))
	  == ffi.cast("ComponentDataTable **", 0) then
		-- Register the component
		io.write(string.format(
				   "The component %s is being registered with %d entries\n",
				   name,
				   component.numEntries))
		C.sceneAddComponentType(scene.ptr, C.idFromName(name), ffi.sizeof(component.type), component.numEntries)
	  end
	end
  end

  -- Find all physics systems in the scene
  io.write("Searching for physics systems\n")
  local systemsFile = io.popen(ffi.os == "Windows" and 'dir /b resources\\scripts\\systems' or 'find resources/scripts/systems -name "*.lua"')
  for line in systemsFile:lines() do
	if ffi.os == "Windows" then
	  line = 'resources/scripts/systems/'..line
	end
	local system = require(string.sub(line, 0, -5))

	if system.init then
	  local err, message = pcall(system.init, scene)
	  if err == false then
		io.write(string.format("Error raised during physics init for system %s\n%s\n",
							   line,
							   message))
	  end
	end

	if system.renderSystem then
	  scene.renderSystems[#scene.renderSystems + 1] = system
	else
	  scene.physicsSystems[#scene.physicsSystems + 1] = system
	end
  end

  --[[
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
	  local err, message = pcall(physicsSystem.init, scene)
	  if err == false then
		io.write(string.format("Error raised during physics init for system %s\n%s\n",
							   ffi.string(ffi.cast("UUID *", itr[0].data).string),
							   message))
	  end
	end

	scene.physicsSystems[i] = physicsSystem

	local itrRef = ffi.new("ListIterator[1]", itr)
	C.listMoveIterator(itrRef)
	itr = itrRef[0]
	i = i + 1
  end
  --]]

  io.write(string.format("Loaded %d physics systems\n", #scene.physicsSystems))

  -- Find all render systems in the scene
  --[[
  list = ffi.new("List[1]", scene.ptr.luaRenderFrameSystemNames)
  itr = ffi.new(
	"ListIterator",
	C.listGetIterator(
	  list))

  io.write("Got rendering iterator\n")

  i = 1
  while C.listIteratorAtEnd(itr) == 0 do
	io.write(string.format("Adding render system number: %d\n", i))
	-- Load all the systems named
	local renderSystem = require(string.format(
								   "resources/scripts/systems/%s",
								   ffi.string(ffi.cast("UUID *", itr[0].data).string)))

	-- Call all the init functions
	if renderSystem.init then
	  local err, message = pcall(renderSystem.init, scene)
	  if err == false then
		io.write(string.format("Error raised during render init for system %s\n%s\n",
							   ffi.string(ffi.cast("UUID *", itr[0].data).string),
							   message))
	  end
	end

	scene.renderSystems[i] = renderSystem

	local itrRef = ffi.new("ListIterator[1]", itr)
	C.listMoveIterator(itrRef)
	itr = itrRef[0]
	i = i + 1
  end
  --]]

  io.write(string.format("Loaded %d render systems\n", #scene.renderSystems))

  engine.scenes[pScene] = scene
end

function engine.runPhysicsSystems(pScene, dt)
  local scene = engine.scenes[pScene]

  local null = ffi.new("int64", 0)
  local emptyUUID = ffi.new("UUID")

  for i = 1,#scene.physicsSystems do
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
			for k = 2,#physicsSystem.components do
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
			  local err, message = pcall(physicsSystem.run,
										 scene,
										 ffi.cast("UUID *", firstComp[0].data
													+ j
													* (firstComp[0].componentSize
														 + ffi.sizeof("UUID")))[0],
										 dt)
			  if err == false then
				io.write(string.format("Error raised while running physics system\n%s\n",
									   message))
			  end
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

  for i = 1,#scene.renderSystems do
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
			for k = 2,#renderSystem.components do
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
			  local err, message = pcall(renderSystem.run,
										 scene,
										 ffi.cast("UUID *", firstComp[0].data
													+ j
													* (firstComp[0].componentSize
														 + ffi.sizeof("UUID")))[0],
										 dt)
			  if err == false then
				io.write(string.format("Error raised while running render system\n%s\n",
									   message))
			  end
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
			 #scene.physicsSystems))

  -- TODO: Call all the shutdown methods on the systems
  for i = 1,#scene.physicsSystems do
	if scene.physicsSystems[i].shutdown then
	  local err, message = pcall(scene.physicsSystems[i].shutdown, scene)
	  if err == false then
		io.write(string.format("Error raised during physics shutdown\n%s\n",
							   message))
	  end
	end
  end

  io.write(string.format(
			 "Scene has %d render systems\n",
			 #scene.renderSystems))

  for i = 1,#scene.renderSystems do
	if scene.renderSystems[i].shutdown then
	  local err, message = pcall(scene.renderSystems[i].shutdown, scene)
	  if err == false then
		io.write(string.format("Error raised during render shutdown\n%s\n",
							   message))
	  end
	end
  end
end
