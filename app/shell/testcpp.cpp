#include <stdio.h>
#include <app.h>
#include <lk/debug.h>
#include <lib/console.h>
// #include <kernel/timer.h>
#include <kernel/thread.h>

template <typename R>
class test_item {
public:
	R get_value() const;
	void set_value(R x);

	bool same_value(const test_item &target) const
	{
		return value == target.value;
	}
	test_item(): value(1), id(2) {}
	test_item(R init_value): value(init_value), id(3){}
	test_item(R init_value, R init_id): value(init_value), id(init_id){}
private:
	R value;
	R id;
};

template<int i>
class B
{
public:
	int num()
	{
		return i * B<i-1>().num();
	}
};

template<>
class B <1>
{
public:
	int num()
	{
		return 1;
	}
};


template<typename T, int i=1>
class someComputing {
public:
	typedef volatile T* retType;
	enum { retValume = i + someComputing<T, i-1>::retValume }; // recursion
	static void f() { printf("someComputing: i=%d\n", i); }
};

template<typename T> // template specialization, end condition
class someComputing<T, 0> {
public:
	enum { retValume = 0 };
};

template<typename T>
class codeComputing {
public:
	static void f() { T::f(); }
};


template <typename R> R test_item<R>:: get_value() const
{
	printf("id is %d \t", id);
	return value;
}

template <typename R> void test_item<R>:: set_value(R x)
{
	value = x;
}

test_item<int> y;

// extern "C"  void testcpp_entry(const struct app_descriptor *app, void *args)
extern "C" void testcpp_entry(void)
{
    test_item<int> x;
    test_item<int> z(4);
    test_item<int> w(5,0);

    int i = B<5>().num();

	someComputing<int>::retType a=0;

	printf("a is %d\n",sizeof(a));
	printf("i:%d\n", i);
	i = someComputing<int, 256>::retValume;
	printf("i is:%d\n", i);
	codeComputing< someComputing<int, 99> >::f();
	printf("x:%d\n", x.get_value());
	printf("y:%d\n", y.get_value());
	x.set_value(3);
	y.set_value(3);
	printf("z:%d\n", z.get_value());
	printf("w:%d\n", w.get_value());

	if (x.same_value(y)) {
		printf("x and y are the same\r\n");
	}
	
}


// APP_START(testcpp)
//   .entry = testcpp_entry,
// APP_END
