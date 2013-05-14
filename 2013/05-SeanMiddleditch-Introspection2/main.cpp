// Copyright (C) 2013 Sean Middleditch
// All rights reserverd.  This code is intended for instructional use only and may not be used
// in any commercial works nor in any student projects.

#include "Meta.h"

#include <cassert>
#include <string>

// a test class
class A1
{
	int a;
	float b;

	int bar(float method) { return (int)(method * 0.5f); }
	float baz(double d, char is_const) { return d > (double)is_const ? (float)(d * 0.5) : 0.f; }

public:
	int getA() const { return a; }
	void setA(int _) { a = _; }

	float getB() const { return b; }

	void foo() { a *= 3; }

	META_DECLARE(A);
};

// another test
class A2
{
	int d;
	std::string name;

public:
	int getD() const { return d; }
	void setD(int _) { d = _; }

	void setName(const std::string& _) { name = _; }
	const std::string& getName() const { return name; }

	void gaz() { assert(d == 0xDEADBEEF); }
};

// another test, with derivation
class B : public A1, public A2
{
	float gar(float m) { return c += m; }

public:
	float c;

	META_DECLARE(B);
};

// a test with extern Get
namespace TestC
{
	struct C
	{
		float x, y;
	};
}

// create m_Type infos
META_DEFINE_EXTERN(std::string);

META_DEFINE(A1)
	.member("a", &A1::a) // note that we can bind private m_Members with this style of Get
	.member("b", &A1::b)
	.member("a2", &A1::getA)
	.member("a3", &A1::getA, nullptr)
	.member("a4", &A1::getA, &A1::setA)
	.method("foo", &A1::foo)
	.method("bar", &A1::bar)
	.method("baz", &A1::baz);

META_DEFINE_EXTERN(A2)
	.member("d", &A2::getD, &A2::setD)
	.member("name", &A2::getName, &A2::setName)
	.method("setName", &A2::setName)
	.method("gaz", &A2::gaz);

META_DEFINE(B)
	.base<A1>()
	.base<A2>()
	.member("c", &B::c)
	.method("gar", &B::gar);

META_DEFINE_EXTERN(TestC::C)
	.member("x", &TestC::C::x)
	.member("y", &TestC::C::y);

// tests that a value can be round-tripped into a member variable
template <typename T, typename U> static void test_rw_member(T& obj, const char* name, const U& value)
{
	auto m = Meta::Get(obj)->FindMember(name);
	assert(m != nullptr);
	Meta::Any a = Meta::make_any(&value);
	assert(m->CanSet(Meta::make_any(&obj), a));
	m->Set(Meta::make_any(&obj), a);
	assert(m->CanGet(Meta::make_any(&obj)));
	U test = Meta::any_cast<U>(m->Get(Meta::make_any(&obj)));
	assert(test == value);
}

// tests that a member variable has a particular value
template <typename T, typename U> static void test_ro_member(T& obj, const char* name, const U& value)
{
	auto m = Meta::Get(obj)->FindMember(name);
	assert(m != nullptr);
	assert(m->CanGet(Meta::make_any(&obj)));
	U test = Meta::any_cast<U>(m->Get(Meta::make_any(&obj)));
	assert(test == value);
}

// tests that a method with no return value or parameters can be invoked
template <typename T> static void test_method(T& obj, const char* name)
{
	auto m = Meta::Get(obj)->FindMethod(name);
	assert(m != nullptr);
	assert(m->CanCall(Meta::make_any(&obj), 0, nullptr));
	m->Call(Meta::make_any(&obj), 0, nullptr);
}

// tests that a method with one parameter results in a specific return value
template <typename T, typename R, typename P> static void test_method(T& obj, const char* name, const R& value, const P& p)
{
	auto m = Meta::Get(obj)->FindMethod(name);
	assert(m != nullptr);
	Meta::Any argv[1] = { Meta::make_any_ref(p) };
	assert(m->CanCall(Meta::make_any(&obj), 1, argv));
	auto r = m->Call(Meta::make_any(&obj), 1, argv);
	assert(r.GetType() == Meta::Get<R>());
	assert(r.GetReference<R>() == value);
}

// tests that a method with two parameters results in a specific return value
template <typename T, typename R, typename P, typename P2> static void test_method(T& obj, const char* name, const R& value, const P& p, const P2& p2)
{
	auto m = Meta::Get(obj)->FindMethod(name);
	assert(m != nullptr);
	Meta::Any argv[2] = { Meta::make_any(&p), Meta::make_any(&p2) };
	assert(m->CanCall(Meta::make_any(&obj), 2, argv));
	auto r = m->Call(Meta::make_any(&obj), 2, argv);
	assert(r.GetType() == Meta::Get<R>());
	assert(r.GetReference<R>() == value);
}

static void test_any()
{
	Meta::Any a;
	const int& ir = 0xDEADC0DE;
	a = Meta::make_any_ref(ir);
	assert(a.GetPointer() == &ir);

	int i = 0xBEEF1337;
	a = Meta::make_any(i);
	assert(a.GetPointer() != &i);
	assert(*reinterpret_cast<int*>(a.GetPointer()) == i);
}

static void test_meta()
{
	B b;
	test_rw_member(b, "a", 12);
	b.setA(31);
	test_ro_member(b, "a2", b.getA());
	b.setA(43);
	test_ro_member(b, "a3", b.getA());
	test_rw_member(b, "a4", -7);
	test_rw_member(b, "b", 191.73f);
	test_rw_member(b, "c", -0.5f);
	test_rw_member(b, "name", std::string("testme"));
	b.setName("hiya");
	test_ro_member(b, "name", std::string("hiya"));
	b.setD(91317);
	test_ro_member(b, "d", b.getD());
	b.setD(0xDEADBEEF);
	test_method(b, "gaz");
	test_method(b, "foo");
	test_ro_member(b, "a", -21);
	test_method(b, "gar", -1.f, -0.5f);
	test_ro_member(b, "c", -1.f);
	test_method(b, "bar", 5, 11.f);
	test_method(b, "baz", 0.f, 5.0, (char)7);
	test_method(b, "baz", 10.f, 20.0, (char)7);
}

// test routine
int main(int argc, char* argv[])
{
	test_any();
	test_meta();
}