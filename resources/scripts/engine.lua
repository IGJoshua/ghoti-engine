ffi = require("ffi")

engine = {}

-- io.output("lua.log")

io.write("Loaded cFFI\n")

engine.C = ffi.load(
  ffi.os == "Windows"
    and "./ghoti.dll"
    or "./ghoti.so")

local C = engine.C

io.write("Loaded ghoti library\n")

engine.kazmath = ffi.load(
  ffi.os == "Windows"
    and "./libkazmath.a"
    or "lualib/libkazmath.so")

io.write("Loaded kazmath library\n")

require("resources/scripts/cdefs")

io.write("Created cffi definitions\n")

engine.input = require("resources/scripts/input")

engine.keyboard = require("resources/scripts/keyboard")
engine.mouse = require("resources/scripts/mouse")
engine.gamepad = require("resources/scripts/gamepad")

local Scene = require("resources/scripts/scene")

io.write("Required scene\n")

engine.components = require("resources/scripts/components")

-- iterate over every file in resources/scripts/components/ and require them
local testFile = io.popen(
  ffi.os == "Windows"
    and 'dir /b resources\\scripts\\components'
    or 'find resources/scripts/components -name "*.lua"')
for line in testFile:lines() do
  if ffi.os == "Windows" then
    line = 'resources/scripts/components/'..line
  end
  require(string.sub(line, 0, -5))
end

engine.scenes = {}
engine.systems = {}

io.write("Searching for systems\n")
local systemsFile = io.popen(
  ffi.os == "Windows"
    and 'dir /b resources\\scripts\\systems'
    or 'find resources/scripts/systems -name "*.lua"')
for line in systemsFile:lines() do
  if ffi.os == "Windows" then
    line = 'resources/scripts/systems/'..line
  end
  local system = require(string.sub(line, 0, -5))
  local systemName = string.sub(line, 27, -5)

  if system.init then
    local err, message = pcall(system.init, scene)
    if err == false then
      io.write(string.format(
                 "Error raised during physics init for system %s\n%s\n",
                 line,
                 message))
    end
  end

  engine.systems[systemName] = system
end

function engine.initScene(pScene)
  local scene = Scene:new(pScene)

  -- Register all lua components into the scene
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
        C.sceneAddComponentType(
          scene.ptr,
          C.idFromName(name),
          ffi.sizeof(component.type),
          component.numEntries)
      end
    end
  end

  engine.scenes[pScene] = scene
end

function engine.runSystems(pScene, dt, physics)
  local scene = engine.scenes[pScene]

  local null = ffi.new("int64", 0)
  local emptyUUID = ffi.new("UUID")

  local itr

  if physics then
    itr = C.listGetIterator(scene.ptr.luaPhysicsFrameSystemNames)
  else
    itr = C.listGetIterator(scene.ptr.luaRenderFrameSystemNames)
  end

  while C.listIteratorAtEnd(itr) == 0 do
    local system = engine.systems[
      ffi.string(ffi.cast("UUID *", itr.curr.data).string)]

    if not system then
      error("Unable to load system, panic")
    end

    if system.begin then
      local err, message = pcall(system.begin, scene, dt)
      if err == false then
        io.write(string.format("Error while beginning a system\n%s\n", message))
      end
    end

    if system.run then
      local componentName = ffi.new(
        "UUID[1]",
        C.idFromName(system.components[1]))
      -- Loop over every entity which has the correct components
      local firstComp = ffi.cast(
        "ComponentDataTable **",
        C.hashMapGetKey(
          scene.ptr.componentTypes,
          componentName))

      if ffi.cast("int64", firstComp) ~= null
      and ffi.cast("int64", firstComp[0]) ~= null then
        for j = 0,tonumber(firstComp[0].numEntries) - 1 do
          if ffi.string(emptyUUID.string) ~= ffi.string(
            ffi.cast("UUID *", firstComp[0].data + j
                       * (firstComp[0].componentSize
                          + ffi.sizeof("UUID"))).string) then

            local valid = true
            for k = 2,#system.components do
              local componentID = C.idFromName(system.components[k])
              local componentTable = ffi.cast(
                "ComponentDataTable **",
                C.hashMapGetKey(
                  scene.ptr.componentTypes,
                  componentID))

              if ffi.cast("int64", componentTable) ~= null
              and ffi.cast("int64", componentTable[0]) then
                if ffi.cast("int64", ffi.cast("uint32 *", C.hashMapGetKey(
                                                componentTable[0].idToIndex,
                                                firstComp[0].data
                                                  + j
                                                  * (firstComp[0].componentSize
                                                       + ffi.sizeof("UUID")))))
                == null then
                  valid = false
                  break
                end
              end
            end

            if valid then
              local err, message = pcall(
                system.run,
                scene,
                ffi.cast("UUID *", firstComp[0].data
                           + j
                           * (firstComp[0].componentSize
                                + ffi.sizeof("UUID")))[0],
                dt)
              if err == false then
                io.write(string.format(
                           "Error raised while running physics system\n%s\n",
                           message))
              end
            end
          end
        end
      end

      if system.clean then
        local err, message = pcall(system.clean, scene, dt)
        if err == false then
          io.write(string.format(
                     "Error raised while calling the clean system\n%s\n",
                     message))
        end
      end
    end

    local itrRef = ffi.new("ListIterator[1]", itr)
    C.listMoveIterator(itrRef)
    itr = itrRef[0]
  end
end

function engine.runPhysicsSystems(pScene, dt)
  engine.runSystems(pScene, dt, true)
end

function engine.runRenderSystems(pScene, dt)
  engine.runSystems(pScene, dt, nil)
end

function engine.shutdownScene(pScene)
  local scene = engine.scenes[pScene]
  engine.scenes[pScene] = nil

  local itr = C.listGetIterator(scene.ptr.luaPhysicsFrameSystemNames)
  while C.listIteratorAtEnd(itr) == 0 do
    local physicsSystem = engine.systems[
      ffi.string(ffi.cast("UUID *", itr.curr.data).string)]
    if physicsSystem.shutdown then
      local err, message = pcall(physicsSystem.shutdown, scene)
      if err == false then
        io.write(string.format("Error raised during physics shutdown\n%s\n",
                               message))
      end
    end

    local itrRef = ffi.new("ListIterator[1]", itr)
    C.listMoveIterator(itrRef)
    itr = itrRef[0]
  end

  itr = C.listGetIterator(scene.ptr.luaRenderFrameSystemNames)
  while C.listIteratorAtEnd(itr) == 0 do
    local renderSystem = engine.systems[
      ffi.string(ffi.cast("UUID *", itr.curr.data).string)]

    if renderSystem.shutdown then
      local err, message = pcall(renderSystem.shutdown, scene)
      if err == false then
        io.write(string.format("Error raised during render shutdown\n%s\n",
                               message))
      end
    end

    local itrRef = ffi.new("ListIterator[1]", itr)
    C.listMoveIterator(itrRef)
    itr = itrRef[0]
  end
end

function engine.cleanInput()
  for key, value in pairs(engine.keyboard) do
    if type(value) == "table" then
      value.updated = false
    end
  end
  for key, value in pairs(engine.mouse.buttons) do
    if type(value) == "table" then
      value.updated = false
    end
  end

  engine.mouse.scroll.x = 0
  engine.mouse.scroll.y = 0

  for name, button in pairs(engine.gamepad.buttons) do
    button.updated = false
  end

  for name, dpad in pairs(engine.gamepad.dpad) do
    dpad.updated = false
  end
end

io.write("Running init script\n")

require("resources/scripts/init")

io.write("Finished init script\n")
