#include "liboo.h"

#include <stdio.h>
#include <stdlib.h>

class(Vec2)
{
    float x, y;
};

void ClassMethod(Vec2, print)
{
    printf("Pos: %f, %f\n", this->x, this->y);
}

void ClassMethod(Vec2, print2params, int id, int not_id)
{
    printf("Pos: %f, %f\n", this->x, this->y);
    printf("unused: %d, %d\n", id, not_id);
}

ClassInit(Vec2)
{
    this->x = 0.0f;
    this->y = 0.0f;
}
EndClassInit()

ClassInit(Vec2, another)
{
    this->x = 10.0f;
    this->y = 10.0f;
}
EndClassInit()

void Callback(const char *pErrorString)
{
    printf("liboo error: %s\n", pErrorString);
}

int main(int argc, char **argv)
{
    SetLibOOErrorCallBack(Callback);
    Vec2 Object = {10.0f, 20.0f};
    Vec2 *pObject = new(Vec2);
    Vec2 *pObject2 = new(Vec2, Vec2_new_another);

    CallMethod(&Object, Vec2_print);
    CallMethod(&Object, Vec2_print2params, 10, 20);

    CallMethod(pObject, Vec2_print);
    CallMethod(pObject, Vec2_print2params, 300, 20);

    CallMethod(pObject2, Vec2_print);
    CallMethod(pObject2, Vec2_print2params, 300, 20);

    delete(pObject);
    delete(pObject2);
    return 0;
}