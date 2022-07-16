import "Core" for Script, Input

class RotatingCube is Script {
    construct new(){

    }

    update(){
        //World.Find('egg').on('collide'){|data| }

        if(Input.is_a_down && Input.is_d_down){
            e.rotate_x(0.5)
        }
        if(Input.is_w_down && Input.is_s_down){
            e.rotate_z(0.5)
        }
        if(Input.is_w_down){
            e.move_z(0.1)
        }
        if(Input.is_s_down){
            e.move_z(-0.1)
        }
        if(Input.is_a_down){
            e.move_x(0.1)
        }
        if(Input.is_d_down){
            e.move_x(-0.1)
        }
    }
}