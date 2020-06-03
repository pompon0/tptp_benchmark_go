#define DEBUG if(1)
#include "gtest/gtest.h"
#include "lazyparam_prover/mgu.h"
#include "lazyparam_prover/util/log.h"

using namespace tableau;

TEST(MGU,flat_loop) {
  StreamLogger _(std::cerr);
  Valuation V;
  Term var0(V.allocate(Var(0)));
  Term var1(V.allocate(Var(0)));
  ASSERT_TRUE(V.mgu(var1,var0));
  ASSERT_TRUE(V.mgu(var0,var1));
  ASSERT_TRUE(!V[0]);
  ASSERT_EQ(V[1].get(),var0);
}

TEST(MGU,nonflat_loop) {
  StreamLogger _(std::cerr);
  Valuation V;
  Term v0(V.allocate(Var(0)));
  Term v1(V.allocate(Var(0)));
  u64 f = 0;
  ASSERT_TRUE(V.mgu(v0,Term(Fun(f,{v1}))));
  ASSERT_FALSE(V.mgu(v1,Term(Fun(f,{v0}))));
}

TEST(MGU,equal) {
  StreamLogger _(std::cerr);
  Valuation V;
  u64 f = 0;
  Term x(V.allocate(Var(0)));
  Term y(V.allocate(Var(0)));
  Term fx(Fun(f,{x}));
  Term ffx(Fun(f,{fx}));
  Term fy(Fun(f,{y}));
  ASSERT_TRUE(V.mgu(y,fx));
  ASSERT_TRUE(V.equal(x,x));
  ASSERT_TRUE(V.equal(y,y));
  ASSERT_TRUE(V.equal(y,fx));
  ASSERT_TRUE(V.equal(fy,ffx));
  ASSERT_FALSE(V.equal(y,x));
  ASSERT_FALSE(V.equal(fy,x));
  ASSERT_FALSE(V.equal(y,ffx));
}

TEST(MGU,eval) {
  StreamLogger _(std::cerr);
  Valuation V;
  Term c(Fun(0,{}));
  Term x(V.allocate(Var(0)));
  ASSERT_TRUE(V.mgu(c,x));

  Atom a(false,5,{c,x});
  Atom b(false,5,{c,c});
  auto ca = AndClause::make(a,b,a);
  auto cb = AndClause::make(b,b,b);
  DerAndClause::Builder builder;
  builder.derived = ca;
  builder.sources = {ca,ca,cb};
  builder.constraints = {OrderAtom::neq(a,b)};
  auto before = builder.build();
  builder.derived = cb;
  builder.sources = {cb,cb,cb};
  builder.constraints = {OrderAtom::neq(b,b)};
  auto got = V.eval(before);
  auto want = builder.build();
  ASSERT_FALSE(before==got);
  ASSERT_EQ(got,want);
}
