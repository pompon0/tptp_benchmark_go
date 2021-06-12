package main

import (
  "context"
  "log"
  "fmt"
  "flag"
  "bytes"
  "time"
  "os"
  "sort"
  "strings"
  "path"
  "os/exec"

  "github.com/google/uuid"
  "github.com/pompon0/tptp_benchmark_go/tool"
  "github.com/pompon0/tptp_benchmark_go/problems"
  "github.com/pompon0/tptp_benchmark_go/utils"
)

const mcts_bin_path = "__main__/ffprover/mcts"

var timeout = flag.Duration("timeout",time.Minute,"")
var priorityModelPath = flag.String("priority_model_path","","")
var rewardModelPath = flag.String("reward_model_path","","")
var caseNamePrefix = flag.String("case_name_prefix","","")
var outputDir = flag.String("output_dir","","")
var solvedInReportPath = flag.String("solved_in_report_path","","")

func outPriorityDir() string { return path.Join(*outputDir,"priority") }
func outRewardDir() string { return path.Join(*outputDir,"reward") }

func MCTS(ctx context.Context, tptpFOFProblem []byte, out Output) error {
  tptpPath,cleanup,err := tool.WriteTmp(tptpFOFProblem)
  if err!=nil { return fmt.Errorf("tool.WriteTmp(): %w",err) }
  defer cleanup()

  problemHash := fmt.Sprintf("%s_%v",time.Now().Format("2006-01-02_15:04:05"),uuid.New())
  cmd := exec.Command(utils.Runfile(mcts_bin_path),
    fmt.Sprintf("--timeout=%v",*timeout),
    fmt.Sprintf("--problem_path=%v",tptpPath),
    fmt.Sprintf("--priority_model_path=%v",*priorityModelPath),
    fmt.Sprintf("--reward_model_path=%v",*rewardModelPath),
    fmt.Sprintf("--priority_training_path=%v",out.PriorityDir.File(problemHash)),
    fmt.Sprintf("--reward_training_path=%v",out.RewardDir.File(problemHash)),
  )
  var inBuf,outBuf bytes.Buffer
  cmd.Stdin = &inBuf
  cmd.Stdout = &outBuf
  cmd.Stderr = os.Stderr
  const memLimitBytes = 2*1000*1000*1000
  gracefulExitTimeout := 10*time.Second
  cmdCtx,cancel := context.WithTimeout(ctx,*timeout+gracefulExitTimeout)
  defer cancel()
  if err := utils.RunWithMemLimit(cmdCtx,cmd,memLimitBytes); err!=nil {
    return fmt.Errorf("cmd.Run(): %w",err)
  }
  return nil
}

func process(ctx context.Context, name string, p *problems.Problem, out Output) error {
  if !strings.HasPrefix(name,*caseNamePrefix) { return nil }
  if problems.SolvedByVampireWithoutEquality[name] { return nil }
  tptp,err := p.Get()
  if err!=nil {
    return fmt.Errorf("p.Get(): %w",err)
  }
  if has,err := tool.TptpHasEquality(ctx,tptp); err!=nil {
    return fmt.Errorf("tool.TptpHasEquality(): %w",err)
  } else if !has {
    return nil
  }
  log.Printf("solving %q",name)
  if err:=MCTS(ctx,tptp,out); err!=nil {
    return fmt.Errorf("MCTS(): %w",err)
  }
  return nil
}

type Dir struct {
  root string
  path string
}

func NewDir(root string, path_ string) (Dir,error) {
  rootStat,err := os.Stat(root)
  if err!=nil { return Dir{},fmt.Errorf("os.Stat(%q): %w",root,err) }
  if !rootStat.IsDir() {
    return Dir{},fmt.Errorf("%q is not a dir",root)
  }
  if err:=os.MkdirAll(path.Join(root,path_),0777); err!=nil {
    return Dir{},fmt.Errorf("os.MkdirAll(): %w",err)
  }
  return Dir{root,path_},nil
}

func (d Dir) File(name string) string {
  return path.Join(d.root,d.path,name)
}

type Output struct {
  PriorityDir Dir
  RewardDir Dir
}

func run(ctx context.Context) error {
  filter := func(name string) bool { return true }
  if p := *solvedInReportPath; p!="" {
    report,err := problems.ReadReport(*solvedInReportPath)
    if err!=nil {
      return fmt.Errorf("problems.ReadReport(): %w",err)
    }
    m := map[string]bool{}
    for _,c := range report.Cases {
      m[c.Name] = c.GetOutput().GetSolved()
    }
    filter = func(name string) bool { return m[name] }
  }

  outPriorityDir,err:=NewDir(*outputDir,"priority")
  if err!=nil { return fmt.Errorf("NewDir(): %w",err) }
  outRewardDir,err:=NewDir(*outputDir,"reward")
  if err!=nil { return fmt.Errorf("NewDir(): %w",err) }

  out := Output{
    PriorityDir: outPriorityDir,
    RewardDir: outRewardDir,
  }

  mp,cancel,err := problems.MizarProblems()
  if err!=nil { return fmt.Errorf("problems.MizarProblems(): %w",err) }
  defer cancel()

  tp,cancel,err := problems.TptpProblems()
  if err!=nil { return fmt.Errorf("problems.TptpProblems(): %w",err) }
  defer cancel()

  type namedProblem struct { name string; p *problems.Problem }
  var ps []namedProblem
  for name,p := range mp {
    ps = append(ps,namedProblem{name,p})
  }
  for name,p := range tp {
    ps = append(ps,namedProblem{name,p})
  }
  // Fix the order for determinism.
  sort.Slice(ps,func(i,j int)bool{ return ps[i].name<ps[j].name })
  for _,p := range ps {
    if !filter(p.name) { continue }
    if err:=process(ctx,p.name,p.p,out); err!=nil {
      return fmt.Errorf("process(%q): %w",p.name,err)
    }
  }
  return nil
}

func main() {
  flag.Parse()
  if err:=run(context.Background()); err!=nil {
    log.Fatalf("run(): %v",err)
  }
}