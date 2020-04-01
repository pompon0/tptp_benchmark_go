#ifndef EQ_AXIOMS_H_
#define EQ_AXIOMS_H_

#include <map>
#include "lazyparam_prover/log.h"
#include "lazyparam_prover/types.h"
#include "lazyparam_prover/syntax/term.h"
#include "lazyparam_prover/syntax/atom.h"
#include "lazyparam_prover/syntax/clause.h"
#include "lazyparam_prover/syntax/show.h"
#include "lazyparam_prover/derived.h"

namespace tableau {

DerOrClause refl_axiom() {
  Term x(Var(0));
  return DerOrClause(0,OrClause({
    Atom::eq(true,x,x)
  }));
}

DerOrClause symm_axiom() {
  Term x(Var(0));
  Term y(Var(1));
  OrClause::Builder b(2);
  b.set_atom(0,Atom::eq(false,x,y));
  b.set_atom(1,Atom::eq(true,y,x));
  return DerOrClause(1,b.build());
}

DerOrClause trans_axiom() {
  Term x(Var(0));
  Term y(Var(1));
  Term z(Var(2));
  return DerOrClause(3,OrClause({
    Atom::eq(false,x,y),
    Atom::eq(false,y,z),
    Atom::eq(true,x,z),
  }));
}


DerOrClause cong_pred_axiom(u64 pred_name, u64 arg_count) {
  Atom::Builder lb(false,pred_name,arg_count);
  Atom::Builder rb(true,pred_name,arg_count);
  OrClause::Builder cb(arg_count+2);
  for(size_t i=0; i<arg_count; ++i) {
    Term la(Var(2*i));
    Term ra(Var(2*i+1));
    cb.set_atom(i,Atom::eq(false,la,ra));
    lb.set_arg(i,la);
    rb.set_arg(i,ra);
  }
  cb.set_atom(arg_count,lb.build()); 
  cb.set_atom(arg_count+1,rb.build());
  return DerOrClause(3,cb.build());
}

DerOrClause cong_fun_axiom(u64 fun_name, u64 arg_count) {
  Fun::Builder lb(fun_name,arg_count);
  Fun::Builder rb(fun_name,arg_count);
  OrClause::Builder cb(arg_count+1);
  for(size_t i=0; i<arg_count; ++i) {
    Term la(Var(2*i));
    Term ra(Var(2*i+1));
    cb.set_atom(i,Atom::eq(false,la,ra));
    lb.set_arg(i,la);
    rb.set_arg(i,ra);
  }
  cb.set_atom(arg_count,Atom::eq(true,Term(lb.build()),Term(rb.build()))); 
  return DerOrClause(3,cb.build());
}

struct ArityCtx {
  ArityCtx(){ pred_arity[Atom::EQ] = 2; }

  std::map<u64,size_t> pred_count;
  std::map<u64,u64> fun_arity;
  std::map<u64,u64> pred_arity;

  void traverse(Term t) {
    switch(t.type()) {
      case Term::VAR: return;
      case Term::FUN: {
        Fun f(t);
        if(fun_arity.count(f.fun()) && fun_arity[f.fun()]!=f.arg_count())
          error("arity mismatch for f% : got % and %",f.fun(),fun_arity[f.fun()],f.arg_count());
        fun_arity[f.fun()] = f.arg_count();
        for(size_t i=f.arg_count(); i--;) traverse(f.arg(i));
        return;
      }
    }
    error("unexpected t.type() = %",t.type());
  }

  void traverse(Atom a) {
    if(!pred_count.count(a.pred())) pred_count[a.pred()] = 0;
    pred_count[a.pred()]++;
    if(pred_arity.count(a.pred()) && pred_arity[a.pred()]!=a.arg_count())
      error("arity mismatch for p%, got % and %",a.pred(),pred_arity[a.pred()],a.arg_count());
    pred_arity[a.pred()] = a.arg_count();
    for(size_t i=a.arg_count(); i--;) traverse(a.arg(i));
    return;
  }

  void traverse(const OrClause &c) { for(size_t i=c.atom_count(); i--;) traverse(c.atom(i)); }
  void traverse(const DerOrClause &c) {
    traverse(c.derived());
    for(size_t i=c.source_count(); i--;) traverse(c.source(i));
  }
  void traverse(const NotAndForm &f) { for(const auto &c : f.or_clauses) traverse(c); }
};

bool has_equality(OrForm f) {
  ArityCtx ctx; ctx.traverse(NotAndForm(f));
  return ctx.pred_count[Atom::EQ]>0;
}

// every (a!=a /\ ...) is subsumed by (a!=a)
OrForm add_refl_constraints(OrForm f) {
  for(auto &c : f.and_clauses) {
    vec<OrderAtom> rc;
    auto d = c.derived();
    for(size_t i=0; i<d.atom_count(); ++i) {
      auto a = d.atom(i);
      if(a.pred()==Atom::EQ && !a.sign()) {
        rc.push_back(OrderAtom(OrderAtom::NE,a.arg(0),a.arg(1)));
      }
    }
    c = c.neg().append_constraints(rc).neg();
  }
  return f;
}


OrForm append_eq_axioms(OrForm _f) {
  // 383/2003 don't use equality axioms (my CNF)
  // 516/2003 use reflexivity only (my CNF)
  // 549/2003 use refl + symm only (my CNF)
  // 740/2003 use refl + symm + mono (my CNF)
  // 640/2003 use refl + symm + trans (my CNF)
  NotAndForm f(add_refl_constraints(_f));
  ArityCtx ctx; ctx.traverse(f);
  f.or_clauses.push_back(refl_axiom());
  f.or_clauses.push_back(symm_axiom());
  f.or_clauses.push_back(trans_axiom());
  for(auto pa : ctx.pred_arity) if(pa.first!=Atom::EQ && pa.second) f.or_clauses.push_back(cong_pred_axiom(pa.first,pa.second));
  for(auto fa : ctx.fun_arity) if(fa.second) f.or_clauses.push_back(cong_fun_axiom(fa.first,fa.second));
  //info("f + axioms = \n%",show(OrForm(f)));
  return OrForm(f);
}


/////////////////////////////////////////////////////////////////////////////

struct FlatClauseBuilder {
  size_t var_count;
  vec<Atom> atoms;
  vec<AndClause> source_clauses;
  size_t cost;

  FlatClauseBuilder(DerAndClause cla) {
    cost = cla.cost();
    var_count = cla.derived().var_range().end;
    for(size_t i=0; i<cla.source_count(); i++) {
      source_clauses.push_back(cla.source(i));
    }
    auto d = cla.derived();
    for(size_t i=d.atom_count(); i--;) flatten_Atom(d.atom(i));
  }

  Var introduce_var(Term t) {
    switch(t.type()) {
      case Term::VAR: {
        source_clauses.push_back(AndClause({Atom::eq(false,t,t)}));
        return Var(t);
      }
      case Term::FUN: {
        Fun fa(t);
        Fun fv = flatten_Term(fa);
        // (f(a1..an)!=f(v1..vn)) (f(a1..an)=f(v1..vn) /\ f(v1..vn)=x /\ f(a1..an)!=x)
        // -> (f(v1..vn)=x /\ f(a1..an)!=x)
        Var x(var_count++);
        auto fa_fv = Atom::eq(true,Term(fa),Term(fv));
        auto fv_x = Atom::eq(true,Term(fv),Term(x));
        auto fa_x = Atom::eq(true,Term(fa),Term(x));
        source_clauses.push_back(AndClause({fa_fv,fv_x,fa_x.neg()}));
        atoms.push_back(fv_x);
        return x;
      }
      default: error("t.type() = %",t.type());
    }
  }

  Fun flatten_Term(Fun fa) {
    // (a1!=v1)..(an!=vn)
    // (a1=v1 /\../\ an=vn /\ f(a1..an)!=f(v1..vn))
    // -> (f(a1..an)!=f(v1..vn))
    vec<Atom> cla;
    Fun::Builder fb(fa.fun(),fa.arg_count());
    for(size_t i=0; i<fa.arg_count(); ++i){
      Var vi = introduce_var(fa.arg(i));
      cla.push_back(Atom::eq(true,fa.arg(i),Term(vi)));
      fb.set_arg(i,Term(vi));
    }
    auto fv = fb.build();
    cla.push_back(Atom::eq(false,Term(fa),Term(fv)));
    source_clauses.push_back(AndClause(cla));
    return fv;
  }

  void flatten_Atom(Atom a) {
    if(a.pred()==Atom::EQ) {
      auto l = a.arg(0);
      auto r = a.arg(1);
      if(l.type()==Term::FUN) {
        // (..l=r) (l=l2 /\ l2=r /\ l!=r)
        Term l2(flatten_Term(Fun(l)));
        source_clauses.push_back(
          AndClause({Atom::eq(true,l,l2),Atom::eq(a.sign(),l2,r),Atom::eq(!a.sign(),l,r)}));
        l = l2;
      }
      if(r.type()==Term::FUN) {
        // (..l=r) (r=r2 /\ l=r2 /\ l!=r)
        Term r2(flatten_Term(Fun(r)));
        source_clauses.push_back(
          AndClause({Atom::eq(true,r,r2),Atom::eq(a.sign(),l,r2),Atom::eq(!a.sign(),l,r)}));
        r = r2;
      }
      atoms.push_back(Atom::eq(a.sign(),l,r));
    } else {
      Atom::Builder b(a.sign(),a.pred(),a.arg_count());
      // (... p(a1..an)) (a1=v1 /\../\ an=vn /\ !p(a1..an) /\ p(v1..vn)) -> (... p(v1..vn))
      vec<Atom> cla;
      for(size_t i=0; i<a.arg_count(); ++i) {
        Var v = introduce_var(a.arg(i));
        cla.push_back(Atom::eq(true,a.arg(i),Term(v)));
        b.set_arg(i,Term(v));
      }
      auto pv = b.build();
      cla.push_back(a.neg());
      cla.push_back(pv);
      source_clauses.push_back(AndClause(cla));
      atoms.push_back(pv);
    }
  }

  DerAndClause build() const {
    DerOrClause::Builder b(source_clauses.size(),0);
    b.set_cost(cost);
    b.set_derived(AndClause(atoms).neg());
    for(size_t i=0; i<source_clauses.size(); ++i)
      b.set_source(i,source_clauses[i].neg());
    return b.build().neg();
  }
};

inline OrForm flatten_OrForm(OrForm f) {
  OrForm f2;
  for(auto cla : f.and_clauses) f2.and_clauses.push_back(FlatClauseBuilder(cla).build());
  return f2;
}

OrForm reduce_monotonicity_and_append_eq_axioms(OrForm _f) {
  NotAndForm f(flatten_OrForm(_f));
  f.or_clauses.push_back(refl_axiom());
  f.or_clauses.push_back(symm_axiom());
  f.or_clauses.push_back(trans_axiom());
  //info("f = \n%",show(_f));
  //info("m(f) + axioms = \n%",show(OrForm(f)));
  return OrForm(f);
}

NotAndForm append_restricted_transitivity_axioms(NotAndForm f) {
  Term a(Var(0));
  Term b(Var(1));
  Term c(Var(2));

  // -[a=b] symm(a,b)
  {
    DerOrClause::Builder symm1(0,0);
    symm1.set_cost(0);
    symm1.set_derived(OrClause({
      Atom(false,Atom::EQ_TRANS_POS,{a,b}),
      Atom(true,Atom::EQ_SYMM,{a,b}),
    }));
    f.or_clauses.push_back(symm1.build());
  }
  // -[b=a] symm(a,b)
  {
    DerOrClause::Builder symm2(1,0);
    symm2.set_cost(0);
    symm2.set_derived(OrClause({
      Atom(false,Atom::EQ_TRANS_POS,{b,a}),
      Atom(true,Atom::EQ_SYMM,{a,b}),
    }));
    symm2.set_source(0,OrClause({
      Atom(false,Atom::EQ,{b,a}),
      Atom(true,Atom::EQ,{a,b}),
    }));
    f.or_clauses.push_back(symm2.build());
  }
  // -symm(a,b) b/=c a=c
  {
    DerOrClause::Builder trans_pos(1,0);
    trans_pos.set_cost(1);
    trans_pos.set_derived(OrClause({
      Atom(false,Atom::EQ_SYMM,{a,b}),
      Atom(false,Atom::EQ,{b,c}),
      Atom(true,Atom::EQ,{a,c}),
    }));
    trans_pos.set_source(0,OrClause({
      Atom(false,Atom::EQ,{a,b}),
      Atom(false,Atom::EQ,{b,c}),
      Atom(true,Atom::EQ,{a,c}),
    }));
    f.or_clauses.push_back(trans_pos.build());
  }
  // {a=b} a/=c b/=c
  {
    DerOrClause::Builder trans_neg(1,0);
    trans_neg.set_cost(0);
    trans_neg.set_derived(OrClause({
      Atom(true,Atom::EQ_TRANS_NEG,{a,b}),
      Atom(false,Atom::EQ,{b,c}),
      Atom(false,Atom::EQ,{a,c}),
    }));
    trans_neg.set_source(0,OrClause({
      Atom(true,Atom::EQ,{a,b}),
      Atom(false,Atom::EQ,{b,c}),
      Atom(false,Atom::EQ,{a,c}),
    }));
    f.or_clauses.push_back(trans_neg.build());
  }
  return f;
}


OrForm append_eq_axioms_with_restricted_transitivity(OrForm _f) {
  NotAndForm f(_f);
  ArityCtx ctx; ctx.traverse(f);
  for(auto pa : ctx.pred_arity) if(pa.first!=Atom::EQ && pa.second) f.or_clauses.push_back(cong_pred_axiom(pa.first,pa.second));
  for(auto fa : ctx.fun_arity) if(fa.second) f.or_clauses.push_back(cong_fun_axiom(fa.first,fa.second));
  for(auto &dc : f.or_clauses) {
    auto c = dc.derived();
    OrClause::Builder b(c.atom_count());
    for(size_t i=c.atom_count(); i--;) {
      Atom a = c.atom(i);
      if(a.pred()==Atom::EQ) {
        Atom::Builder ab(a.sign(),a.sign()?Atom::EQ_TRANS_POS:Atom::EQ_TRANS_NEG,a.arg_count());
        for(size_t j=a.arg_count(); j--;) ab.set_arg(j,a.arg(j));
        b.set_atom(i,ab.build());
      } else b.set_atom(i,a);
    }
    DerOrClause::Builder db(dc.source_count(),dc.constraint_count());
    db.set_cost(dc.cost());
    db.set_derived(b.build());
    for(size_t i=dc.source_count(); i--;) db.set_source(i,dc.source(i));
    for(size_t i=dc.constraint_count(); i--;) db.set_constraint(i,dc.constraint(i));
    dc = db.build();
  }
  f.or_clauses.push_back(refl_axiom());
  f = append_restricted_transitivity_axioms(f);
  //info("f + axioms = \n%",show(OrForm(f)));
  return OrForm(f);
}


/////////////////////////////////////////////////////////////////////////////

struct Index {
  struct OrClauseWithAtom { size_t i; DerOrClause cla; };
  static size_t atom_hash(Atom a) { return (a.pred()-Atom::PRED_MIN)<<1|a.sign(); }
private:
  vec<OrClauseWithAtom> empty;
  vec<vec<vec<OrClauseWithAtom>>> map; // atom -> cost -> OrClauseWithAtom
public:
  Index(const NotAndForm &f) { FRAME("Index");
    for(auto cla : f.or_clauses) {
      //DEBUG info("cla.derived.atom_count() = %",cla.derived().atom_count());
      for(size_t i=0; i<cla.derived().atom_count(); ++i) {
        //DEBUG info("Index i=%",i);
        auto h = atom_hash(cla.derived().atom(i));
        //DEBUG info("h = %",h);
        if(map.size()<=h) map.resize(h+1,{{}});
        //DEBUG info("cla.cost() = %",cla.cost());
        if(map[h].size()<=cla.cost()) map[h].resize(cla.cost()+1,map[h].back());
        for(size_t cc=cla.cost(); cc<map[h].size(); ++cc) map[h][cc].push_back({i,cla});
      }
    }
  }

  // find all atoms with same pred and opposite sign
  const vec<OrClauseWithAtom>& operator()(Atom a, size_t max_cost) const {
    auto h = atom_hash(a)^1;
    return h>=map.size() ? empty : max_cost<map[h].size() ? map[h][max_cost] : map[h].back();
  }
};

} // namespace tableau

#endif  // EQ_AXIOMS_H_
