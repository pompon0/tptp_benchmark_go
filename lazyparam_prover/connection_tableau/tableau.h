#ifndef CONNECTION_TABLEAU_TABLEAU_H_
#define CONNECTION_TABLEAU_TABLEAU_H_

#include "lazyparam_prover/connection_tableau/cont.h"
#include "lazyparam_prover/memory/function.h"

namespace tableau::connection_tableau {

DEBUG_ONLY(
struct Mutex {
  struct Locked {
    ~Locked(){ m->locked = false; }
    Mutex *m;
  };
  [[nodiscard]] Locked lock(){
    if(locked) error("already locked");
    locked = true;
    return {this};
  }
private:
  bool locked = false;
};)

struct ActionCollector {
  ActionCollector(SearchState *_state) : state(_state) {}
  SearchState *state;
  // std::function uses heap allocation, we should avoid it.
  template<typename F> [[nodiscard]] INL bool diverge(memory::Alloc &A, F f) { FRAME("ActionCollector::diverge");
    static_assert(memory::has_sig<F,memory::Maybe<TaskSet>(void)>());
    DEBUG_ONLY(auto l = diverging.lock();)
    auto s = state->save();
    actions.push(A,f());
    state->restore(s);
    return false;
  }
  memory::List<memory::Maybe<TaskSet>> actions;
private:
  DEBUG_ONLY(Mutex diverging;)
};

struct ActionExecutor {
  ActionExecutor(SearchState *_state, int _skip_count) : state(_state), skip_count(_skip_count) {}
  SearchState *state;
  template<typename F> [[nodiscard]] INL bool diverge(memory::Alloc &A, F f) { FRAME("ActionExecutor::diverge");
    static_assert(memory::has_sig<F,memory::Maybe<TaskSet>(void)>());
    DEBUG_ONLY(auto l = diverging.lock();)
    if(skip_count--) return false;
    if(auto mts = f()) {
      task_set = mts.get();
    } else {
      error("task set not found");
    }
    return true;
  }
  TaskSet get() {
    DEBUG if(skip_count!=-1) error("action not found");
    return task_set;
  }
private:
  DEBUG_ONLY(Mutex diverging;)
  int skip_count;
  TaskSet task_set;
};

static Task start_task(memory::Alloc &A) {
  return Task(Task::Start::Builder(A).build());
}

// Search takes alloc as an argument to be able to return result in its memory.
alt::SearchResult search(const Ctx &ctx, memory::Alloc &A, SearchState &state, size_t depth_limit) { FRAME("connection_tableau::search()");
  SCOPE("connection_tableau::search");
  struct Save {
    TaskSet cont; // always execute the last
    SearchState::Save ss;
    memory::Alloc::Save As;
    memory::List<memory::Maybe<TaskSet>> actions; // actions left
  };
  auto task = start_task(A);
  auto ss = state.save();
  TaskSet cont(A,task);
  ActionCollector ac(&state);
  task.run(A,&ac);
  vec<Save> saves{Save{cont,ss,A.save(),ac.actions}};
  
  size_t steps = 0;
  for(; saves.size(); steps++) {
    // select action
    if(saves.back().actions.empty()){ saves.pop_back(); continue; }
    auto s = saves.back();
    saves.back().actions = saves.back().actions.tail();
    if(!s.actions.head()) continue;
    if(steps%100==0 && ctx.done()) return {0,steps};
    DEBUG if(steps%1000==0) info("steps = %",steps);
    
    // execute action
    A.restore(s.As);
    state.restore(s.ss);
    ActionExecutor ae(&state,s.actions.size()-1);
    s.cont.head().run(A,&ae);
    auto ss = state.save();
    
    // push back new tasks
    TaskSet ts = s.cont.tail();
    bool ok = true;
    for(auto nt = ae.get(); !nt.empty(); nt = nt.tail()) {
      // skip action if proof tree depth has been exceeded
      if(nt.head().features().depth>depth_limit) ok = false;
      ts.push(A,nt.head());
    }
    if(ts.empty()) { return {1,steps}; }
    if(!ok) continue;

    // determine next actions
    ActionCollector ac(&state);
    ts.head().run(A,&ac);
    saves.push_back(Save{ts,ss,A.save(),ac.actions});
  }
  DEBUG info("steps = %",steps);
  return {0,steps};
}

static ProverOutput prove(const Ctx &ctx, memory::Alloc &A, const ClauseIndex &cla_index, const FunOrd &fun_ord, size_t limit) { FRAME("prove()");
  SCOPE("prove");
  SearchState s(cla_index,fun_ord);
  auto res = search(ctx,A,s,limit);
  s.stats.val = s.val.stats;
  DEBUG_ONLY(
    if(res.found) {
      str trace;
      size_t i = 0;
      for(auto l=s.trace; !l.empty(); l=l.tail()) {
        trace = util::fmt("[%] %\n",i++,l.head()) + trace;
      }
      info("TRACE = \n%",trace);
    }
  )
  return {
    res.cont_count,
    limit,
    s.val.get_valuation(),
    res.found ? s.get_proof(A) : 0,
    s.stats,
  };
}

static ProverOutput prove_loop(const Ctx &ctx, memory::Alloc &A, OrForm form, const FunOrd &fun_ord) { FRAME("prove_loop()");
  SCOPE("prove_loop"); 
  Stats stats;
  size_t cont_count = 0;
  size_t limit = 0;
  //info("ClauseIndex begin");
  ClauseIndex idx(form);
  //info("ClauseIndex end");
  for(;!ctx.done();) {
    limit++; // avoid incrementing limit before context check
    DEBUG info("limit = %",limit);
    ProverOutput out = prove(ctx,A,idx,fun_ord,limit);
    out.cont_count += cont_count;
    out.stats += stats;
    if(out.proof) {
      DEBUG info("SUCCESS");
      DEBUG info("%",show(*out.proof));
      return out;
    }
    stats = out.stats;
    cont_count = out.cont_count;
    //std::cerr << "expands[" << limit << "]: " << profile.scopes["expand"].count << std::endl;
  }
  DEBUG info("FAILURE");
  ProverOutput out;
  out.cont_count = cont_count;
  out.cost = limit;
  out.stats = stats;
  return out; 
}

} // namespace connection_tableau::tableau

#endif  // CONNECTION_TABLEAU_TABLEAU_H_
