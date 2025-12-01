#include "liboo.h"

#include <stdio.h>
#include <stdlib.h>

class(Vec2)
{
    float x, y;
};

void NewFunction(Vec2, print)
{
    printf("Pos: %f, %f\n", this->x, this->y);
}

void NewFunctionEx(Vec2, print2params, int id, int not_id)
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

void Callback(const char *pErrorString)
{
    printf("liboo error: %s\n", pErrorString);
}

int main(int argc, char **argv)
{
    SetLibOOErrorCallBack(Callback);
    Vec2 Object = {10.0f, 20.0f};
    Vec2 *pObject = new(Vec2);
    CallFunction(&Object, Vec2, print);
    CallFunctionEx(&Object, Vec2, print2params, 10, 20);
    CallFunction(pObject, Vec2, print);
    CallFunctionEx(pObject, Vec2, print2params, 300, 20);
    delete(pObject);
    return 0;
}