#ifndef CONNECTION_TABLEAU_FRAMES_WEAK_H_
#define CONNECTION_TABLEAU_FRAMES_WEAK_H_

#include "lazyparam_prover/connection_tableau/frames/strong.h"

// exec (S->S) -> (And [S->S] | Or [S->S])
//   executes a single step of a subtree, of a function which is defined coinductively 

// template<typename D> void weak(D *d) { ... }
// struct D {
//   memory::Alloc &A,
//   SearchState *state;
//   const size_t size_limit;
//   void done(){}
//   void and_(fun<void(D*)> f){}
//   void or_(fun<void(D*)> f){}
// }
struct _WeakFrame {
  Branch branch;

  template<typename DState> INL void run(DState *d) const {
    STATE_FRAME(d->A,d->state,"weak(%)",show(branch.false_.head()));
    d->state->stats.weak_steps++;
    // try to match with lemma
    auto a = branch.false_.head();
    auto atom_hash = Index::atom_hash(a);
    Features f{.depth=branch.false_.size()};
    for(auto b = branch.true_; !b.empty(); b = b.tail()) {
      if(atom_hash!=Index::atom_hash(b.head())) continue;
      if(!d->state->val.equal_mod_sign(a,b.head())) continue;
      d->done(f);
      return;
    }
    if(!a.strong_only()) {
      // try to unify with path
      for(auto b = branch.false_; !b.empty(); b = b.tail()) {
        if((atom_hash^1)!=Index::atom_hash(b.head())) continue;
        d->or_(f,[f,a,b](DState *d)INLL{
          if(!d->state->val.unify(d->A,a,b.head())) return;
          d->done(f);
        });
      }
    }
    
    auto br = branch;
    d->or_(f,[f,a,br](DState *d)INLL{
      if(!a.strong_only()) {
        // add constraints (wrt path)
        for(auto b = br.false_.tail(); !b.empty(); b = b.tail()) {
          if(!d->state->val.push_constraint(d->A,OrderAtom::neq(d->A,a,b.head()))) return;
        }
      }
      
      // extend
      auto matches = d->state->cla_index.get_matches(br.false_.head(),memory::just(d->size_limit - d->state->nodes_used));
      while(auto mca = matches.next()) {
        auto ca = mca.get();
        d->or_(f,[br,ca](DState *d)INLL{ strong(d,br,ca.cla,ca.i); });
      }
    });
  }
};

#endif  // CONNECTION_TABLEAU_FRAMES_WEAK_H_
