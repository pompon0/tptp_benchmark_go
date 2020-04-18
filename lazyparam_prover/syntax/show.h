#ifndef SYNTAX_SHOW_H_
#define SYNTAX_SHOW_H_

#include "lazyparam_prover/syntax/term.h"
#include "lazyparam_prover/syntax/atom.h"
#include "lazyparam_prover/syntax/clause.h"
#include "lazyparam_prover/types.h"
#include "lazyparam_prover/util/string.h"

namespace tableau {

str show(Term t) {
  switch(t.type()) {
    case Term::VAR: return util::fmt("V%",Var(t).id());
    case Term::FUN: {
      Fun fun(t);
      switch(fun.fun()) {
        case Fun::EXTRA_CONST: return "c";
        case Fun::VAR_WRAP: return "v";
        case Fun::FUN_WRAP: return "f";
      }
      vec<str> args(fun.arg_count());
      for(size_t i=0; i<fun.arg_count(); ++i) args[i] = show(fun.arg(i));
      return util::fmt("f%(%)",Fun(t).fun(),util::join(",",args));
    }
  }
  error("unexpected t.type() = %",t.type());
}

str show_pred_name(u64 pred) {
  switch(pred) {
  case Atom::EQ: return "eq";
  case Atom::EQ_TRANS_POS: return "[eq]";
  case Atom::EQ_TRANS_NEG: return "{eq}";
  case Atom::EQ_SYMM: return "symm";
  case Atom::TRANS_TARGET: return "trans_target";
  case Atom::TRANS_RED: return "trans_red";
  case Atom::MONO_RED: return "mono_red";
  }
  return util::fmt("p%",pred);
}

str show(Atom a) {
  vec<str> args(a.arg_count());
  for(size_t i=a.arg_count(); i--;) args[i] = show(a.arg(i));
  str sign = a.sign() ? "+" : "-";
  str pred_name = show_pred_name(a.pred());
  return util::fmt("%%(%)",sign,pred_name,util::join(",",args));
}

str show(const OrClause cla) {
  vec<str> atoms;
  for(size_t i=0; i<cla.atom_count(); ++i) atoms.push_back(show(cla.atom(i)));
  return util::join(" \\/ ",atoms);
}

str show(const AndClause cla) {
  vec<str> atoms;
  for(size_t i=0; i<cla.atom_count(); ++i) atoms.push_back(show(cla.atom(i)));
  return util::join(" /\\ ",atoms);
}

} // namespace tableau

#endif // SYNTAX_SHOW_H_
