# Class Configuration Language (CCL)
CCL是为C/C++设计的一种标记语言，目前C语言的运行时解析库正在开发，它目前拥有以下标准类型:

int32, int64, float32, float64, bool, string, list, array, set, time_t

同时它还支持C语言的注释方法，`/**/`与`//`，在计划中它将可以被翻译为C++结构体和JSON，也可以解析为字节码，但更推荐使用解析库直接调用。

array和set必须以以下方式定义:
```
array<int32> aaa = [1, 1, 2, 3, 3, 4, 4]; // 任何类型都可以放入<>
set<int32> bbb = (1, 2, 3, 4); // 同上，但由于是集合，后面元素不能相同
```
list则允许存放任意类型，如下:
```
/*
    aaa是刚刚的array<int32> aaa
    bbb是刚刚的set<int32> bbb
*/
list ccc = {&aaa, &bbb, "Hello CCL!"};
```

这些在结构外的数据会被直接初始化允许调用。

它还允许你定义自己的结构，这些结构都会直接被初始化为单例，除非使用interface定义而非class:

```
class WeaponHammer
{
    string id = "hammer";
    int32 max_ammo = -1; // -1 = unlimited
    int32 reload_timer = 150; // in ms
    int32 ammo_regen = 0; // 0 = disabled
};

class WeaponGun
{
    string id = "gun";
    int32 max_ammo = 10;
    int32 reload_timer = 200; // in ms
    int32 ammo_regen = 250; // in ms
};

class Weapons
{
    list data = {&WeaponHammer, &WeaponGun};
    array<bool> is_enabled = [true, true]; // this is a array
};
```

当然, CCL也支持继承与导入（但是不能多继承，基类必须是interface，interface可以继承另一个interface）, 因此上方还可以这样写：

weaponbase.ccl:
```
// interface 不能直接被调用为配置
interface WeaponBase 
{
    string id = "unknown"
    int32 max_ammo = 0;
    int32 reload_timer = 0; // in ms
    int32 ammo_regen = 0; // in ms
}
```
weapons.ccl:
````
#include "weaponbase.ccl"

class WeaponHammer from WeaponBase
{
    id = "hammer"; // 这里直接覆盖了原id, 你不能在子类里定义同名配置
    max_ammo = -1; // -1 = unlimited
    reload_timer = 150;
};

class WeaponGun from WeaponBase
{
    id = "gun";
    max_ammo = 10;
    reload_timer = 200; // in ms
    ammo_regen = 250; // in ms
};

class Weapons
{
    set<WeaponBase> data = [&WeaponHammer, &WeaponGun];
    array<bool> is_enabled = [true, true]; // this is a array
};
```
需要注意的是, CCL的数组/列表/集合元素在实现中，不带&应当不被视为引用，而是拷贝。

```
class NotToday
{
    time_t timestamp = 1717020800; // Unix时间戳
};

class Event1
{
    time_t event_time = "2025-12-01T12:00:00Z"; // UTC
};

class Event2
{
    time_t event_time = "2025-12-01T12:00:00+08:00" // UTC+8
};
```