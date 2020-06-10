#define DEBUG
#include "lazyparam_prover/lpo.h"
#include "lazyparam_prover/util/log.h"
#include "lazyparam_prover/rewrite_test_utils.h"
#include "lazyparam_prover/constrained_valuation.h"
#include "gtest/gtest.h"

using namespace tableau;

TEST(LPO,reduction_ordering) {
  StreamLogger _(std::cerr);
  reduction_ordering_test_suite<LPO>();
}

TEST(LPO,simple) {
  StreamLogger _(std::cerr);
  ConstrainedValuation<LPO> lpo;
  Term x(lpo.allocate(Var(0)));
  Term y(lpo.allocate(Var(0)));
  TestCtx ctx(7895374);
  auto f = ctx.new_fun<Term,Term>();
  auto g = ctx.new_fun<Term>(); 
  auto h = ctx.new_fun<Term>();

  EXPECT_EQ(OrderAtom::E,lpo.cmp(x,x));
  EXPECT_EQ(OrderAtom::L,lpo.cmp(x,h(x)));
  EXPECT_EQ(OrderAtom::G,lpo.cmp(h(x),x));
  EXPECT_EQ(OrderAtom::U,lpo.cmp(x,h(y)));
  EXPECT_EQ(OrderAtom::L,lpo.cmp(x,f(y,x)));
  EXPECT_EQ(OrderAtom::L,lpo.cmp(g(x),f(y,g(x))));
  EXPECT_EQ(OrderAtom::U,lpo.cmp(g(g(g(g(y)))),f(y,g(x))));
  EXPECT_EQ(OrderAtom::U,lpo.cmp(f(y,g(x)),g(g(g(g(y))))));
  EXPECT_EQ(OrderAtom::L,lpo.cmp(f(y,g(x)),f(y,h(x))));
}

TEST(LPO,incompleteness) {
  StreamLogger _(std::cerr);
  TestCtx ctx(7589354);
  auto f = ctx.new_fun<Term,Term>();
  for(size_t cases=10; cases--;) {
    ConstrainedValuation<LPO> lpo;
    Term x(lpo.allocate(Var(0)));
    Term y(lpo.allocate(Var(0)));
    auto a = f(x,y);
    auto b = f(y,f(x,x));
    // case 0: unevaluated a,b -> U
    // default: ground a,b -> L
    auto want = OrderAtom::U;
    if(cases!=0) {
      random_valuate(ctx,lpo,4,true);
      want = OrderAtom::L;
    }
    auto got = lpo.cmp(a,b);
    ASSERT_EQ(want,got) << util::fmt("cmp(%,%) = %, want %",show(lpo.eval(a)),show(lpo.eval(b)),got,want);
  }
}