#pragma once

//==========================================
//         BASE OF ALL SCENES
//==========================================

class SceneBase 
{
public:
    // I dont need the construct because its pure a virtual class
    //virtual ~SceneBase() {} // since there is not construct, I can do {} because nothing will be destructed
                            // right now I dont have .cpp for this

    virtual bool Init() = 0;    
    virtual void Update(float deltaTime) = 0;
    virtual void Draw() = 0;
    virtual void Uninit() = 0;
};