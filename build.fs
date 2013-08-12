(* Copyright (C) 2013 Ben Lewis <benjf5+bitbucket@gmail.com>
 * Under the LGPLv3+ license
 *
 * A build-automation system for various languages
 *)

module Build
 
type language = C | Cplusplus | OCaml | Fsharp | Java | Csharp

type compilation_info =
  {
    lang : language;
    options : string;
    output : string;
    files : string list;
  }


  (* This is a dangerous function to hard-code, it would be better to run a
  setup and load compiler names in some fashion. *)
let get_compiler = function
  | C -> "clang"
  | Cplusplus -> "clang++"
  | OCaml -> "ocamlc"
  | Fsharp -> "fs"
  | Java -> "javac"
  | Csharp -> "cs"

let build compl =
  let compiler = (get_compiler compl.lang) in
  let build_string = (compiler + " " + compl.options + " " + "-o " +
    compl.output  + (List.fold (fun x y -> x + " " + y) "" compl.files)) in
  let build_proc = new System.Diagnostics.Process() in
  build_proc.StartInfo.FileName <- build_string;
  build_proc.StartInfo.CreateNoWindow <- true;
  build_proc.StartInfo.RedirectStandardOutput <- true;
  build_proc.StartInfo.UseShellExecute <- false;
  ignore (build_proc.Start ());
  let output = build_proc.StandardOutput.ReadToEnd () in
  build_proc.WaitForExit ();
  output

(* Settings parser *)

let find_language (lang_name : string) =
  if ((lang_name.CompareTo "C") = 0) then Some(C)
  elif (((lang_name.CompareTo "C++") = 0)
        || ((lang_name.CompareTo "Cplusplus") = 0)) then Some(Cplusplus)
  elif ((lang_name.CompareTo "OCaml") = 0) then Some(OCaml)
  elif (((lang_name.CompareTo "F#") = 0)
        || ((lang_name.CompareTo "FSharp") = 0)) then Some(Fsharp)
  elif ((lang_name.CompareTo "Java") = 0) then Some(Java)
  elif (((lang_name.CompareTo "C#") = 0)
        || ((lang_name.CompareTo "CSharp") = 0)) then Some(Csharp)
  else None

