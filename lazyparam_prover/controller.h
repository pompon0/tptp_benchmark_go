#ifndef CONTROLLER_H_
#define CONTROLLER_H_

#include <memory>
#include "lazyparam_prover/index.h"
#include "lazyparam_prover/parse.h"
#include "lazyparam_prover/search_state.h"
#include "lazyparam_prover/connection_tableau/cont.h"
#include "tool/bin/wrapper.h"

// TODO: apart running ML to choose the right alternative to find the answer,
//   run ML to minimize the search space by an appropriate choice of task order
namespace controller {

class Problem {
public:
  using Ptr = std::shared_ptr<const Problem>;
  static Ptr New(const str &tptp_fof) {
    auto file = tptp_to_proto(tptp_cnf(tptp_fof));
    auto p = own(new Problem());
    p->A = make<memory::Alloc>();
    p->idx = make<tableau::ClauseIndex>(tableau::ParseCtx().parse_orForm(*p->A,file));
    return p;
  }
  friend class Prover;
private:
  Problem() = default;
  ptr<memory::Alloc> A;
  ptr<tableau::ClauseIndex> idx;

  static str tptp_cnf(const str &tptp_fof) {
    tool::Request req;
    req.mutable_fof_to_cnf()->set_tptp_fof(tptp_fof);
    return tool::bin::wrapper(Ctx::background(),req).fof_to_cnf().tptp_cnf();
  }
  static tptp::File tptp_to_proto(const str &tptp_cnf) {
    tool::Request req;
    req.mutable_tptp_to_proto()->set_tptp(tptp_cnf);
    req.mutable_tptp_to_proto()->set_lang(tptp::Input::CNF);
    return tool::bin::wrapper(Ctx::background(),req).tptp_to_proto().file();
  }
};

class Prover {  
  struct Div;
  using Task = memory::function<void(Div*)>;
  using Cont = memory::List<Task>;

public:
  struct Action {
    Cont cont;
    tableau::SearchState::Save ss;
    memory::Alloc::Save As;
    bool done() const { return cont.empty(); }
  };
private:
  struct Div {
    enum { size_limit = 1000000 };
    template<typename F> INL void or_(tableau::Features x, F f){ FRAME("or_"); save(_and.add(A,Task(A,f))); }
    template<typename F> INL void and_(F f){ FRAME("and_"); _and.push(A,Task(A,f)); }
    INL void done(tableau::Features f){ save(_and); }
    
    void save(Cont cont) {
      actions.push_back({
        .cont = cont,
        .ss = state->save(),
        .As = A.save(),
      });
    }
    memory::Alloc &A;
    tableau::SearchState *state;

    Cont _and;
    vec<Action> actions;
  };

  Prover() = default;
  ptr<memory::Alloc> A;
  ptr<tableau::SearchState> state; 
  ptr<Action> start;

public:
  static ptr<Prover> New(Problem::Ptr problem) {
    auto p = own(new Prover());
    p->A = make<memory::Alloc>();
    p->state = make<tableau::SearchState>(*problem->idx,FunOrd());
    Cont cont(*p->A,Task(*p->A,[](Div *d){ tableau::connection_tableau::Cont::start(d); }));
    p->start = own(new Action{
      .cont = cont,
      .ss = p->state->save(),
      .As = p->A->save(),
    });
    return p; 
  }

  Action reset() { return *start; }

  vec<Action> run(Action a) {
    DEBUG if(a.cont.empty()) error("cont done");
    A->restore(a.As);
    state->restore(a.ss);
    Div d{.A = *A, .state = state.get(), ._and = a.cont.tail()};
    a.cont.head()(&d);
    return d.actions;
  }
};

bool search(const Ctx &ctx, Prover &p) { FRAME("controller::search");
  SCOPE("controller::search");
  vec<Prover::Action> as{p.reset()};
  size_t steps = 0;
  for(;as.size(); steps++) {
    Prover::Action a = as.back();
    as.pop_back();
    if(a.done()) return true;
    if(steps%100==0 && ctx.done()) break;
    DEBUG if(steps%1000==0) info("steps = %",steps);
    for(auto x : p.run(a)) as.push_back(x);
  }
  DEBUG info("steps = %",steps);
  return false;
}

} // namespace controller

#endif // CONTROLLER_H_
