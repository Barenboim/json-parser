[English version](/README.md)
# 基于标准C(C99)的JSON解析器
项目最初是为[C++ Workflow](https://github.com/sogou/workflow)项目开发的JSON解析。
# 编译项目测试代码
~~~sh
$ make
~~~
# 运行测试代码
### JSON解析与JSON结构复制，序列化测试
~~~sh
$ ./parse_json < xxx.json
~~~
### 解析速度测试
~~~sh
$ time ./test_speed <重复次数> < xxx.json
~~~
# 主要接口
### JSON value相关接口
~~~c
/* 解析JSON文档产生JSON value。返回JSON value对象。返回NULL代表解析失败（格式不标准，嵌套过深，分配内存失败）
   @doc 文档字符串 */
json_value_t *json_value_parse(const char *doc);

/* 销毁JSON value
   @val JSON value对象。一般由parse函数产生。*/
void json_value_destroy(json_value_t *val);

/* 返回JSON value类型
    可能的返回值：
    JSON_VALUE_STRING：字符串
    JSON_VALUE_NUMBER：数字
    JSON_VALUE_OBJECT：json对象
    JSON_VALUE_ARRAY：json数组
    JSON_VALUE_TRUE：true
    JSON_VALUE_FALSE：false
    JSON_VALUE_NULL：null
   @val：JSON value对象 */
int json_value_type(const json_value_t *val);

/* 获得JSON string。如果返回NULL，代表value不是STRING型
   @val：JSON value对象 */
const char *json_value_string(const json_value_t *val);

/* 获得JSON number。如果value不是NUMBER型，返回NAN（不存在的浮点数）
   @val：JSON value对象 */
double json_value_number(const json_value_t *val);

/* 获得JSON object。如果value不是OBJECT类型，返回NULL
   @val：JSON value对象
   注意返回的json_object_t指针并非const。可以通过build相关函数扩展object。*/
json_object_t *json_value_object(const json_value_t *val);

/* 获得JSON array。如果value不是ARRAY类型，返回NULL
   @val：JSON value对象
   同样，返回的json_array_t指针不带const。 */
json_array_t *json_value_array(const json_value_t *val);

~~~
### JSON object相关接口
~~~c
/* 返回object的大小。即object包含的name，value对数量
   @obj：JSON object对象 */
int json_object_size(const json_object_t *obj);

/* 查找并返回name下的value。返回NULL代表找不到这个name。函数时间复杂度为O(log(size))
   @name：要查找的名字
   @obj：JSON object对象
   注意返回的json_value_t指针带const。*/
const json_value_t *json_object_find(const char *name, const json_object_t *obj);

/* 向前或向后遍历JSON object
   @name：临时的const char *类型name字符串
   @val：临时的const json_value_t *类型的JSON value对象
   @obj：JSON object对象
   这是两个宏，会被展开成for循环。 */
json_object_for_each(name, val, obj)
json_object_for_each_prev(name, val, obj)
~~~
### JSON array相关接口
~~~c
/* 返回JSON array的大小，即元素个数
   @arr：JSON array对象 */
int json_array_size(const json_array_t *arr);

/* 向前或向后遍历JSON array
   @val：临时的const json_value_t *类型的JSON value对象
   @arr：JSON array对象
   同样，这是展开成for循环的宏。 */
json_array_for_each(val, arr)
json_array_for_each_prev(val, arr)
~~~

### JSON building相关接口
以下的函数用于JSON编辑，这些函数都可能返回NULL表示内存分配失败。
~~~c
/* 新建一个JSON value
   @type: JSON value的类型
   注意这是一个可变参数的函数，示例:
     v_str = json_value_create(JSON_VALUE_STRING, "hello");
     v_num = json_value_create(JSON_VALUE_NUMBER, 3.14);
     v_int = json_value_create(JSON_VALUE_NUMBER, (double)100);
     v_obj = json_value_create(JSON_VALUE_OBJECT);
     v_arr = json_value_create(JSON_VALUE_ARRAY);
     v_true = json_value_create(JSON_VALUE_TRUE);
   函数返回一个JSON value对象。非const，因此需要被销毁。 */
json_value_t *json_value_create(int type, ...);

/* 扩展一个JSON object，返回被扩展的JSON value
   @obj: JSON object对象
   @name: 扩展的JSON member的name
   @type: 扩展的JSON member类型，或者传0代表扩展进一个已有的JSON value
   注意这是一个可变参数的函数，示例:
     v_num = json_object_append(obj, "pi", JSON_VALUE_NUMBER, 3.14);
     v_obj = json_object_append(obj, "user", JSON_VALUE_OBJECT);
     v_from_doc = json_object_append(obj, "doc", 0, json_value_parse("{\"data\" : [1, 2, 3]}")); */
const json_value_t *json_object_append(json_object_t *obj, const char *name,
                                       int type, ...);

/* 从JSON object里移除一个value并返回
   @val: 要移除的JSON value
   @obj: JSON object对象
   注意，返回值非const，需要被销毁，也可以被扩展到其它的JSON object或JSON array */
json_value_t *json_object_remove(const json_value_t *val,
                                 json_object_t *obj);

/* 扩展JSON array，返回被扩展的JSON value
   @arr: JSON array对象
   @type: 扩展的JSON member类型，或者传0代表传入一个已有的JSON value
   注意这是一个可变参数的函数，示例:
     v_str = json_array_append(arr, JSON_VALUE_STRING, "hello");
     等价于：
     v_str = json_array_append(arr, 0, json_value_create(JSON_VALUE_STRING, "hello")); */
const json_value_t *json_array_append(json_array_t *arr, int type, ...);

/* 从JSON array里移除一个JSON value并返回
   @val: 要移除的JSON value
   @arr: JSON array对象
   注意，返回值非const，需要被销毁，也可以被扩展到其它的JSON object或JSON array */
json_value_t *json_array_remove(const json_value_t *val,
                                json_object_t *arr);
~~~

# 一个优雅的第三方C++封装
https://github.com/wfrest/Json
