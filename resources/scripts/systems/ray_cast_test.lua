io.write("Loading the Ray Cast Test system\n")

local system = {}

local C = engine.C
local kazmath = engine.kazmath

system.init = nil
system.shutdown = nil

system.components = {}
system.components[1] = "rigid_body"
system.components[2] = "transform"
--system.components[3] = ""

local rigid_body
local transform

function system.run(scene, uuid, dt)
    --[[
    --]]

    if (ffi.string(uuid.string) == "ray_caster") then

        transform = scene:getComponent("transform", uuid)
        rigid_body = scene:getComponent("rigid_body", uuid)

        local dir = ffi.new("kmVec3[1]")

        kazmath.kmQuaternionGetForwardVec3RH(dir, transform.globalRotation)

        local collision = C.rayCast(scene.ptr, transform.position, dir[0], 2, 20)

        if(collision.hasContact ~= 0) then
            io.write(string.format("%d\n", tonumber(collision.hasContact)))
        end

    end
end
io.write("Finished loading the Ray Cast Test system\n")

return system
