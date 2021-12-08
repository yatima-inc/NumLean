import Lake
open System Lake DSL

def leanSoureDir := "lib"
def cppCompiler := "c++"
def cppDir : FilePath := "cpp"
def ffiSrc := cppDir / "ffi.cpp"
def ffiO := "ffi.o"
def ffiLib := "libffi.a"

def ffiOTarget (pkgDir : FilePath) : FileTarget :=
  let oFile := pkgDir / defaultBuildDir / cppDir / ffiO
  let srcTarget := inputFileTarget <| pkgDir / ffiSrc
  fileTargetWithDep oFile srcTarget fun srcFile => do
    compileO oFile srcFile
      #["-I", (← getLeanIncludeDir).toString] cppCompiler

def cLibTarget (pkgDir : FilePath) : FileTarget :=
  let libFile := pkgDir / defaultBuildDir / cppDir / ffiLib
  staticLibTarget libFile #[ffiOTarget pkgDir]

package NumLean (pkgDir) {
  srcDir := leanSoureDir
  libRoots := #[`NumLean]
  moreLibTargets := #[cLibTarget pkgDir]
}