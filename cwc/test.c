#include <stdio.h>
#include <stdlib.h>

typedef struct Vec2 Vec2;
struct Vec2 {
        float x, y;
};

void Vec2_print(Vec2 *this)    {
        printf("Vec2: %f, %f\n", this->x, this->y);
    }
Vec2* Vec2_new_default()
{
    Vec2* this = (Vec2*)malloc(sizeof(Vec2));
    if (!this) return NULL;
    {
        this->x = x;
        this->y = y;
    }
    return this;
}

Vec2* Vec2_new_int_int(int x, int y)
{
    Vec2* this = (Vec2*)malloc(sizeof(Vec2));
    if (!this) return NULL;
    {
        this->x = (float)x;
        this->y = (float)y;
    }
    return this;
}


int main()
{
    float fx = 1.0f, fy = 2.0f;
    int ix = 10, iy = 20;

    Vec2 *v1 = Vec2_new_default();                 // → default
    Vec2 *v2 = Vec2_new_default();         // → float_float
    Vec2 *v3 = Vec2_new_int_int(ix, iy);         // → int_int

    v1->print();
    v2->print();
    v3->print();

    { free(v1); v1 = NULL; };
    { free(v2); v2 = NULL; };
    { free(v3); v3 = NULL; };
    return 0;
}