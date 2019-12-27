#ifndef PRED_H_
#define PRED_H_

#include "lazyparam_prover/types.h"
#include "lazyparam_prover/alloc.h"

namespace tableau {

struct Term {
private:
  friend struct Var;
  friend struct Fun;
  friend struct Atom;
  enum { TYPE, SIZE };
  u64 *ptr;
  u64 var_offset;
  Term(u64 *_ptr, u64 _var_offset) : ptr(_ptr), var_offset(_var_offset) {}
public:
  enum Type { VAR, FUN };
  Type type(){ return Type(ptr[TYPE]); }
};

struct Var {
private:
  enum { ID = Term::SIZE, SIZE };
  Term term;
public:
  explicit Var(Term t) : term(t) {
    DEBUG if(term.ptr[Term::TYPE]!=Term::VAR) error("Var(<type=%>)",term.ptr[Term::TYPE]);
  }
  explicit operator Term() { return term; }
  u64 id(){ return term.ptr[ID]+term.var_offset; }
  
  static Var make(u64 id) {
    COUNTER("Var::make");
    auto ptr = alloc(SIZE);
    ptr[Term::TYPE] = Term::VAR;
    ptr[ID] = id;
    return Var(Term(ptr,0));
  }
};

struct Fun {
private:
  enum { FUN = Term::SIZE, ARG_COUNT, ARGS };
  Term term;
public:
  enum { EXTRA_CONST = -1 };
  Fun(Term t) : term(t) {
    DEBUG if(term.ptr[Term::TYPE]!=FUN) error("Fun(<type=%>)",term.ptr[Term::TYPE]);
  }
  explicit operator Term(){ return term; }
  u64 fun(){ return term.ptr[FUN]; }
  u64 arg_count(){ return term.ptr[ARG_COUNT]; }
  Term arg(size_t i) {
    DEBUG if(i>=arg_count()) error("<arg_count=%>.arg(%)",arg_count(),i);
    return Term((u64*)term.ptr[ARGS+i],term.var_offset);
  }
  
  static Fun slow_make(u64 fun, const vec<Term> &args) {
    Builder b(fun,args.size());
    for(size_t i=0; i<args.size(); ++i) b.set_arg(i,args[i]);
    return b.build();
  }

  struct Builder {
  private:
    u64 *ptr;
  public:
    Builder(u64 _fun, u64 _arg_count) : ptr(alloc(ARGS+_arg_count)) {
      COUNTER("Fun::Builder");
      ptr[Term::TYPE] = FUN;
      ptr[FUN] = _fun;
      ptr[ARG_COUNT] = _arg_count;
    }
    void set_arg(size_t i, Term a){
      DEBUG if(a.var_offset) error("Fun::Builder.set_arg(): var_offset = %, want 0",a.var_offset);
      ptr[ARGS+i] = (u64)a.ptr;
    }
    Fun build(){ return Fun(Term(ptr,0)); }
  };
};

struct Atom {
private:
  friend struct OrClause;
  enum { SIGN, PRED, ARG_COUNT, ARGS };
  u64 *ptr;
  u64 var_offset;
  u64 id_; // used to identify atom (for indexing)
  explicit Atom(u64 *_ptr, u64 _var_offset, u64 _id) : ptr(_ptr), var_offset(_var_offset), id_(_id) {}
public:
  enum {
    EQ = u64(-1),
    EQ_TRANS_POS = u64(-2),
    EQ_TRANS_NEG = u64(-3),
    EQ_SYMM = u64(-4),
    PRED_MIN = EQ_SYMM,
  };

  inline bool sign() const { return ptr[SIGN]; }
  inline u64 pred() const { return ptr[PRED]; }
  inline u64 arg_count() const { return ptr[ARG_COUNT]; }
  inline Term arg(size_t i) const { return Term((u64*)ptr[ARGS+i],var_offset); }
  inline u64 id() const { return id_; } 

  static inline Atom eq(bool sign, Term l, Term r) {
    Builder b(sign,EQ,2);
    b.set_arg(0,l);
    b.set_arg(1,r);
    return b.build();
  }

  static inline Atom slow_make(bool sign, u64 pred, const vec<Term> &args) {
    Builder b(sign,pred,args.size());
    for(size_t i=0; i<args.size(); ++i) b.set_arg(i,args[i]);
    return b.build();
  }

  struct Builder {
  private:
    u64 *ptr;
  public:
    Builder(bool _sign, u64 _pred, u64 _arg_count) : ptr(alloc(ARGS+_arg_count)) {
      COUNTER("Atom::Builder");
      ptr[SIGN] = _sign;
      ptr[PRED] = _pred;
      ptr[ARG_COUNT] = _arg_count;
      DEBUG for(size_t i=0; i<_arg_count; ++i) ptr[ARGS+i] = 0;
    }
    inline void set_arg(size_t i, Term a){
      DEBUG if(a.var_offset!=0) error("Atom::set_arg(): var_offset = %, want %",a.var_offset,0);
      ptr[ARGS+i] = (u64)a.ptr;
    }
    inline Atom build(){
      DEBUG for(size_t i=0; i<ptr[ARG_COUNT]; ++i) if(!ptr[ARGS+i]) error("Atom::build() arg(%) not set",i);
      return Atom(ptr,0,0);
    }
  };

  Atom neg() const {
    u64 *end = ptr+ARGS+arg_count();
    u64 *ptr2 = alloc(end-ptr);
    for(auto x = ptr, y = ptr2; x<end;) *y++ = *x++;
    ptr2[SIGN] = !ptr2[SIGN];
    return Atom(ptr2,var_offset,id_);
  }
};

//TODO: replace with hash consing
inline bool operator==(Term x, Term y) {
  if(x.type()!=y.type()) return 0;
  switch(x.type()) {
  case Term::FUN: {
    Fun fx(x),fy(y);
    if(fx.fun()!=fy.fun()) return 0;
    DEBUG if(fx.arg_count()!=fy.arg_count())
      error("fx.arg_count() = %, fy.arg_count() = %",fx.arg_count(),fy.arg_count());
    for(size_t i=fx.arg_count(); i--;)
      if(!(fx.arg(i)==fy.arg(i))) return 0;
    return 1;
  }
  case Term::VAR:
    return Var(x).id()==Var(y).id();
  default:
    error("Term<type=%> == Term<type=%>",x.type(),y.type());
  }
}

inline bool operator!=(Term x, Term y) { return !(x==y); }

inline bool operator==(Atom x, Atom y) {
  if(x.pred()!=y.pred()) return 0;
  DEBUG if(x.arg_count()!=y.arg_count())
    error("x.arg_count() = %, y.arg_count() = %",x.arg_count(),y.arg_count());
  if(x.sign()!=y.sign()) return 0;
  for(size_t i=x.arg_count(); i--;)
    if(!(x.arg(i)==y.arg(i))) return 0;
  return 1;
}

inline bool operator!=(Atom x, Atom y) { return !(x==y); }

struct OrClause;
struct AndClause;
struct NotAndForm;
struct OrForm;

struct AndClause {
  AndClause(size_t _var_count = 0) : var_count(_var_count) {}
  size_t var_count;
  vec<Atom> atoms;
  OrClause neg() const;
};

struct OrClause {
private:
  enum { ATOM_COUNT, VAR_COUNT, ATOMS };
  u64 *ptr;
  u64 var_offset_;
  u64 id_offset_;
  OrClause(u64 *_ptr, u64 _var_offset, u64 _id_offset) : ptr(_ptr), var_offset_(_var_offset), id_offset_(_id_offset) {}
public:
  size_t var_count() const { return var_offset_+ptr[VAR_COUNT]; }
  size_t atom_count() const { return ptr[ATOM_COUNT]; } 
  Atom atom(size_t i) const {
    DEBUG if(i>=atom_count()) error("<atom_count=%>.arg(%)",atom_count(),i);
    return Atom((u64*)ptr[ATOMS+i],var_offset_,id_offset_+i);
  }
  AndClause neg() const;

  OrClause shift(size_t _var_offset) const { FRAME("OrClause::shift()");
    DEBUG if(var_offset_!=0) error("var_offset = %, want %",var_offset_,0);
    return OrClause(ptr,_var_offset,id_offset_);
  }
  OrClause set_id_offset(size_t _id_offset) const { FRAME("OrClause::set_id_offset()");
    DEBUG if(id_offset_!=0) error("id_offset = %, want %",id_offset_,0);
    return OrClause(ptr,var_offset_,_id_offset);
  }

  struct Builder {
  private:
    u64 *ptr;
  public:
    Builder(u64 _atom_count, u64 _var_count) : ptr(alloc(ATOMS+_atom_count)) {
      ptr[VAR_COUNT] = _var_count;
      ptr[ATOM_COUNT] = _atom_count;
    }
    void set_atom(size_t i, Atom a) { FRAME("OrClause.Builder.set_atom()");
      DEBUG if(a.var_offset) error("a.var_offset = %, want %",a.var_offset,0);
      ptr[ATOMS+i] = (u64)a.ptr;
    }
    OrClause build(){ return OrClause(ptr,0,0); }
  };

  bool operator==(const OrClause &cla) const {
    if(cla.atom_count()!=atom_count()) return 0;
    for(size_t i=ptr[ATOM_COUNT]; i--;) if(cla.atom(i)!=atom(i)) return 0;
    return 1;
  }
  bool operator!=(const OrClause &cla) const { return !(*this==cla); }
};

inline OrClause AndClause::neg() const {
  OrClause::Builder b(atoms.size(),var_count);
  for(size_t i=atoms.size(); i--;) b.set_atom(i,atoms[i].neg());
  return b.build();
}

inline AndClause OrClause::neg() const {
  AndClause d(var_count());
  for(size_t i=0; i<atom_count(); ++i) d.atoms.push_back(atom(i).neg());
  return d;
}

struct DerAndClause;
struct DerOrClause;

struct DerAndClause {
  DerAndClause(){}
  explicit DerAndClause(size_t _cost, AndClause cla) : cost(_cost), derived(cla), source{cla} {}

  DerOrClause neg() const;
  
  size_t cost = 0;
  AndClause derived;
  vec<AndClause> source;
};

struct DerOrClause {
  DerOrClause(size_t _cost, OrClause cla) : DerOrClause(_cost,cla,List<OrClause>(cla)) {}
  DerOrClause(size_t _cost, OrClause _derived, List<OrClause> _source) : DerOrClause(_cost,0,0,_derived,_source) {}
  DerAndClause neg() const;
  DerOrClause shift(size_t _var_offset) const {
    DEBUG if(var_offset_) error("offset = %, want %",var_offset_,0);
    return DerOrClause(cost_,_var_offset,id_offset_,derived_,source_);
  }
  DerOrClause set_id_offset(u64 _id_offset) {
    DEBUG if(id_offset_!=0) error("id_offset = %, want %",id_offset_,0);
    return DerOrClause(cost_,var_offset_,_id_offset,derived_,source_);
  }
  OrClause derived() const { return derived_.shift(var_offset_).set_id_offset(id_offset_); }
  size_t cost() const { return cost_; }
  List<OrClause> source_list() {
    //TODO: shift the thing for consistency
    return source_;
  }
  vec<OrClause> source() const {
    vec<OrClause> s; for(auto l=source_; !l.empty(); l = l.tail()) s.push_back(l.head().shift(var_offset_));
    return s;
  }
private:
  DerOrClause(size_t _cost, size_t _var_offset, size_t _id_offset, OrClause _derived, List<OrClause> _source)
    : cost_(_cost), var_offset_(_var_offset), id_offset_(_id_offset), derived_(_derived), source_(_source) {}
  size_t cost_;
  size_t var_offset_;
  size_t id_offset_;
  OrClause derived_;
  List<OrClause> source_;
};

DerOrClause DerAndClause::neg() const {
  List<OrClause> source_neg;
  for(const auto &c : source) source_neg += c.neg();
  return DerOrClause(cost,derived.neg(),source_neg);
}

DerAndClause DerOrClause::neg() const {
  DerAndClause cla;
  cla.cost = cost();
  cla.derived = derived().neg();
  for(auto c : source()) {
    cla.source.push_back(c.neg());
  }
  return cla;
}

struct OrForm {
  vec<DerAndClause> and_clauses;
  OrForm(){}
  explicit OrForm(const NotAndForm &);
};

struct NotAndForm {
  vec<DerOrClause> or_clauses;
  NotAndForm(){}
  explicit NotAndForm(const OrForm &);
};

inline NotAndForm::NotAndForm(const OrForm &f) {
  for(const auto &c : f.and_clauses) or_clauses.push_back(c.neg());
}

inline OrForm::OrForm(const NotAndForm &f) {
  for(const auto &c : f.or_clauses) and_clauses.push_back(c.neg());
}

struct ProverOutput {
  size_t cont_count;
  size_t cost;
  ptr<DerAndClause> proof;
};

static_assert(sizeof(u64*)==sizeof(u64));
static_assert(sizeof(Term)==2*sizeof(u64));
static_assert(sizeof(Var)==sizeof(Term));
static_assert(sizeof(Fun)==sizeof(Term));
static_assert(sizeof(Atom)==sizeof(Term)+sizeof(u64));
static_assert(sizeof(OrClause)==sizeof(Atom));

}  // namespace tableau

#endif // PRED_H_
