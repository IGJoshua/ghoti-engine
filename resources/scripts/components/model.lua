ffi.cdef[[
typedef struct model_component_t
{
    char name[64];
    bool visible;
} ModelComponent;
]]

local component = engine.components:register("model", "ModelComponent")

local C = engine.C

function component:swap(name, staticModel, scene, uuid, removeSkeleton, skeleton, idleAnimation, speed, transitionDuration)
  self.name = name
  C.loadModel(name)

  if scene then
	if staticModel then
	  if removeSkeleton then
		local animation = scene:getComponent("animation", uuid)
		if animation ~= nil then
		  animation:removeSkeleton(scene)
		end
	  end

	  scene:removeComponentFromEntity("animation", uuid)
      scene:removeComponentFromEntity("animator", uuid)
      scene:removeComponentFromEntity("next_animation", uuid)
	else
	  local animator = scene:getComponent("animator", uuid)
	  if animator ~= nil then
		C.removeAnimator(animator)
		C.resetAnimator(animator, nil)
		animator.speed = 1.0
		animator.previousAnimation = ""
		animator.previousAnimationTime = 0.0
		animator.transitionDuration = 0.0
	  end

	  local next_animation = scene:getComponent("next_animation", uuid)
	  if next_animation ~= nil then
		next_animation:set("")
	  end

      local animationComponent = true

      local animation = scene:getComponent("animation", uuid)
      if animation == nil then
        animationComponent = false
        animation = ffi.new("AnimationComponent")
      end

      removeSkeleton = removeSkeleton or false

	  if removeSkeleton and animationComponent then
		animation:removeSkeleton(scene)
      end

      animation.skeleton = skeleton
      animation.idleAnimation = idleAnimation
      animation.speed = speed or 1.0
      animation.transitionDuration = transitionDuration or 0.0

      if not animationComponent then
        scene:addComponentToEntity("animation", uuid, animation)
      end
    end
  end
end

function component:swapMaterial(mesh, material)
  C.swapMeshMaterial(self.name, mesh, material)
end