var Mod = {
	"name": "My Mod",
	"description": "It's a really cool mod!",
	"version": "1.0",
	"dependencies": [
		"modname1",
		"modname2",
	]
}

class TestGravity is Behaviour {
	construct new(){
		
		on('interact', {
			System.write("Interact!")
			Animate.go(2.0, {|t|
				entity.rotation.y = t*Math.PI/2.0
			})
		});

	}
	update(dt) {
		entity.position.y = entity.position.y - 1.0*dt
	}
}