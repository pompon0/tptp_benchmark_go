#ifndef FFPROVER_SEARCH_H_
#define FFPROVER_SEARCH_H_

#include <functional>
#include "lazyparam_prover/controller.h"
#include "lazyparam_prover/features.h"
#include "ffprover/tree.h"
#include "utils/log.h"

namespace ff {

struct Result {
  enum Status { SOLVED, DEADEND, CANCELLED };
  Status status;
  struct Node {
    features::StateVec state;
    vec<features::ActionVec> actions;
    Tree::Ptr tree;
  };
  vec<Node> path;
};

struct Search {
  struct Config { 
    bool one_expansion_per_playout;
    size_t playouts_per_bigstep;
    size_t playout_depth;
    std::function<double(features::StateVec)> base_reward; // [0,1]
    std::function<double(features::ActionVec)> base_priority; 
    std::function<double(Tree::Ptr,size_t)> child_priority;
    std::function<size_t(Tree::Ptr)> bigstep_selector;
  };

  struct Stats {
    size_t inferences = 0;
    size_t bigsteps = 0;
  };

  Search(Config _cfg) : cfg(_cfg) {}

  Config cfg;
  Stats stats;

  Result run(Ctx::Ptr ctx, Tree::Ptr t, controller::Prover &p) { FRAME("Search::run");
    Result res;
    while(!ctx->done()) {
      if(stats.bigsteps%1==0) info("bigsteps = %",stats.bigsteps);
      // push a node on the result path
      res.path.push_back({
        .state = p.state_features(),
        .tree = t,
      });
      if(!p.done()) {
        for(size_t i=0,ac=p.action_count(); i<ac; i++)
          res.path.back().actions.push_back(p.action_features(i));
      }
      // early exit
      if(p.done()){ res.status = Result::SOLVED; return res; }
      if(t.lost()){ res.status = Result::DEADEND; return res; }
      // perform playouts
      auto s = p.save();
      for(size_t i = 0; i<cfg.playouts_per_bigstep; i++) {
        playout(t,p);
        p.restore(s);
      }
      auto i = cfg.bigstep_selector(t);
      //info("bigstep %",i);
      stats.bigsteps++;
      t = t.child(i);
      p.apply_action(i);
    }
    res.status = Result::CANCELLED;
    return res;
  }
  
private:
  double reward(Tree::Ptr t, controller::Prover &p) {
    if(p.done()) return 1.;
    if(t.lost()) return 0.;
    auto r = cfg.base_reward(p.state_features());
    DEBUG if(r>1. || r<0.) error("cfg.base_reward() = %, want in [0,1]",r);
    return r;
  }

  void expand(Tree::Ptr t, controller::Prover &p) {
    size_t n = p.action_count();
    vec<double> priorities(n);
    for(size_t i=0; i<n; i++) priorities[i] = cfg.base_priority(p.action_features(i));
    t.expand(priorities);
  }

  size_t select_child(Tree::Ptr t) {
    if(t.child_count()==0) error("t has no children");
    ssize_t best_i=-1; double best = -1;
    for(size_t i=0; i<t.child_count(); i++) {
      if(t.child(i).lost()) continue;
      auto x = cfg.child_priority(t,i);
      if(best_i==-1 || x>best){ best_i = i; best = x; }
    }
    if(best_i==-1) error("all children lost");
    return size_t(best_i);
  }

  void playout(Tree::Ptr t, controller::Prover &p) {
    size_t expansions = 0;
    //str path = "";
    for(size_t d = cfg.playout_depth; d--;) {
      if(t.lost()) return;
      if(p.done()) break;
      if(!t.expanded()) {
        if(expansions && cfg.one_expansion_per_playout) break;
        expansions++;
        expand(t,p);
        if(t.lost()) break;
      }
      DEBUG if(t.lost()) error("playout in a lost branch");
      auto i = select_child(t);
      //path += util::fmt("%/% ",i,t.child_count());
      t = t.child(i);
      p.apply_action(i);
      stats.inferences++;
    }
    t.visit(reward(t,p));
    if(p.done()) t.set_won();
  }
};

}  // namespace ff

#endif  // FFPROVER_SEARCH_H_