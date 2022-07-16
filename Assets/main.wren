
//Test Vec3

class Vec3 {
    construct new(x, y, z){
        _x = x
        _y = y
        _z = z
    }
    x { _x }
    y { _y }
    z { _z }
    +(o){ Vec3.new(_x+o.x, _y+o.y, _z+o.z) }
    static update(){
        for(i in 1..1000) {
            L.add(Vec3.new(1,2,3) + Vec3.new(3,2,1))
        }
    }
}

/*
class Coin is Component {
    construct new(entity) {
        ECS.GetTransform(entity);
    }
}*/