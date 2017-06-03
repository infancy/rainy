#2017
##5.23
秉承着不要重复造轮子以及专注于解决主要矛盾的精神，放弃了全部自己编写的想法，首先把pbrt的数学几何部分整合到了valley里，为了尽量少的引入文件，
注释掉了error.cpp的#include "progressreporter.h"，transform.cpp的#include "interaction.h"。同时为了方便起见修改了pbrt文件的命名空间。
##5.24
