
foreign class Entity {
    construct new() {}
    foreign rotate_x(angle)
    foreign rotate_y(angle)
    foreign rotate_z(angle)

    foreign move_x(angle)
    foreign move_y(angle)
    foreign move_z(angle)
}

class Script {
    e { _e }
    e=(v) { _e = v }

    /*
    on(event, cb) {
        event_listeners[event].push(cb)
        return cb
    }
    remove_on(event, cb) {
        
    }
    event_listeners{{
        "collide": [CLOSURE, CLOSURE, CLOSURE],
        ""
    }}
    emit_event(event, data) {
        listeners = event_listeners[event]
        for(l in listeners) {
            l.call(data)
        }
    }
    */
}

class Engine {
    foreign static GetTime()
}

class Input {
    foreign static is_w_down
    foreign static is_s_down
    foreign static is_a_down
    foreign static is_d_down
}

/*
class ECS {}
class Physics {}
class Audio {}

foreign class Vec3 {
    foreign x
    foreign y
    foreign z
    construct new() {}
    construct new(x, y, z) { 
        this.x = x
        this.y = y
        this.z = z
    }
    foreign x=(v)
    foreign y=(v)
    foreign z=(v)
/*
    foreign -
    foreign - (other)
    foreign + (other)
*/
}
*/