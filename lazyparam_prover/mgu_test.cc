#define DEBUG if(1)
#include "gtest/gtest.h"
#include "lazyparam_prover/mgu.h"
#include "lazyparam_prover/util/log.h"

using namespace tableau;

TEST(MGU,flat_loop) {
  StreamLogger _(std::cerr);
  Term var0(Var::make(0));
  Term var1(Var::make(1));
  Valuation V; V.resize(2);
  ASSERT_TRUE(V.mgu(var1,var0));
  ASSERT_TRUE(V.mgu(var0,var1));
  ASSERT_TRUE(!V[0]);
  ASSERT_EQ(V[1].get(),var0);
}

TEST(MGU,nonflat_loop) {
  StreamLogger _(std::cerr);
  Valuation V; V.resize(2);
  Term v0(Var::make(0));
  Term v1(Var::make(1));
  u64 f = 0;
  ASSERT_TRUE(V.mgu(v0,Term(Fun::slow_make(f,{v1}))));
  ASSERT_FALSE(V.mgu(v1,Term(Fun::slow_make(f,{v0}))));
}

TEST(MGU,equal) {
  StreamLogger _(std::cerr);
  Valuation V; V.resize(2);
  u64 f = 0;
  Term x(Var::make(0));
  Term y(Var::make(1));
  Term fx(Fun::slow_make(f,{x}));
  Term ffx(Fun::slow_make(f,{fx}));
  Term fy(Fun::slow_make(f,{y}));
  ASSERT_TRUE(V.mgu(y,fx));
  ASSERT_TRUE(V.equal(x,x));
  ASSERT_TRUE(V.equal(y,y));
  ASSERT_TRUE(V.equal(y,fx));
  ASSERT_TRUE(V.equal(fy,ffx));
  ASSERT_FALSE(V.equal(y,x));
  ASSERT_FALSE(V.equal(fy,x));
  ASSERT_FALSE(V.equal(y,ffx));
}
