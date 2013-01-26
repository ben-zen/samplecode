(* Let's make a DL manager with a web interface! *)
(* Copyright (C) 2013 Ben Lewis <benjf5+bitbucket@gmail.com>,
   <benjf5+github@gmail.com> *)
(* Under the LGPLv3 license. *)

(* Uses OcURL and libcurl. *)

(* Uses Unix, Str, String, Buffer, Digest (for now), Curl, and Thread. *)

(* To compile to bytecode: `ocamlc -thread -o dlman unix.cma curl.cma str.cma
   threads.cma dlman.ml`.  Once I get it worked out better, I'll work up a
   native code command. *)

(* Requires opening str.cma and curl.cma; curl.cma provides a strange problem
   in that it currently doesn't properly install. *)

(* type file_info = Digest.t * Thread.t *)

let determine_progress file_digest connection =
  let total_complete = Curl.get_sizedownload connection
  and total_size = Curl.get_contentlengthdownload connection in
  let current_complete =  string_of_float ((total_complete /. total_size )
  *. 100.0) in
  print_string ("{ file_digest : \"" ^ file_digest ^
                   "\", complete: " ^ current_complete ^ " }\n")
  (* This will be replaced with sending that to the web page.
  *)
  
let write_data fp data_buffer data_line =
  let data_length = String.length data_line in 
  Buffer.add_string data_buffer data_line;
  Buffer.output_buffer fp data_buffer;
  Buffer.reset data_buffer;
  data_length

let write_wrapper file_digest connection fp data_buffer data_line =
  determine_progress file_digest connection;
  write_data fp data_buffer data_line
    
let dl_file file_location =
  try 
    let location_list = Str.split (Str.regexp "/") file_location in
    let file_name = List.hd (List.rev location_list) in
    let file_p = open_out_bin file_name in
    let file_digest = Digest.string file_location in
    let incoming_data = Buffer.create 4096 (* using a simple buffer at the
                                              moment. This will become configurable. *)
    and connection = Curl.init () in
    Curl.set_writefunction connection (write_wrapper file_digest connection file_p incoming_data);
    Curl.set_followlocation connection true;
    Curl.set_url connection file_location;
    Curl.perform connection;
    Curl.cleanup connection;
    print_string ("Download complete: " ^ file_name);
    close_out file_p
  with Curl.CurlException (_, _, s) -> print_string s

(* Needs functions to produce a webpage, now that it downloads files. *)

let add_download file_location =
  Thread.create dl_file file_location
      
    (* This function will need to become slightly more complicated, and it will
  need a function around dl_file, since we'll be handling sending output to a
  webpage (if launched), etc. *)

(* let acquire_lock () =
  Unix.chdir download_dir;
  if not (Sys.file_exists "./.dlman.daemon.pid")
  then
    try
      0
    with Sys_error(n) ->
      (print_string ("Could not start the daemon.\n"); 1);
        
  else
    (ignore (print_string ( "The lock file, .dlman.daemon.pid, exists in the download " ^
      "directory specified; if there is no currently-running dlman instance, " ^
      "erase the lock file and start dlman again." ));
     1)
      (* In this case, the daemon will not start.  No other action need be
       * performed. *)    
*)
(* Necessary functionality includes a way to add a download to a
 * currently-running dlman instance, and a web view. Probably also a list of
 * active downloads, and later a list of completed downloads.
 *)

let send_job file_location =
  try
    let dlman_add_sock = Unix.socket Unix.PF_UNIX Unix.SOCK_DGRAM 0 in
    Unix.bind dlman_add_sock (Unix.ADDR_UNIX("/tmp/dlman_add_socket"));
    Unix.connect dlman_add_sock (Unix.ADDR_UNIX("/tmp/dlman_socket"));
    let msg_len = String.length file_location in
    let char_mask = (lnot ((lnot 0) lsl 8)) in
    let msg_len_str = String.make 4 '0' in
    msg_len_str.[0] <- (char_of_int (char_mask land msg_len));
    msg_len_str.[1] <- (char_of_int (char_mask land (msg_len lsr 8)));
    msg_len_str.[2] <- (char_of_int (char_mask land (msg_len lsr 16)));
    msg_len_str.[3] <- (char_of_int (char_mask land (msg_len lsr 24)));
    ignore (Unix.send dlman_add_sock msg_len_str 0 4 []);
    ignore (Unix.send dlman_add_sock file_location 0 msg_len []);
    Unix.close dlman_add_sock;
    Sys.remove "/tmp/dlman_add_sock"
  with Unix.Unix_error (err, cmd, loc) ->
    (print_string "Failed to send job to daemon.";
     Sys.remove "/tmp/dlman_add_sock")
      
let read_loop download_dir =
  try
    let dlman_sock = Unix.socket Unix.PF_UNIX Unix.SOCK_DGRAM 0 in
    Unix.bind dlman_sock (Unix.ADDR_UNIX("/tmp/dlman_socket"));
      try
        while true do
          let msg_size_str = String.create 4 in
          let received_chars = Unix.recv dlman_sock msg_size_str 0 4 [] in
          let msg_size = ((int_of_char msg_size_str.[0]) +
                             ((int_of_char msg_size_str.[1]) lsl 8) +
                             ((int_of_char msg_size_str.[2]) lsl 16) + 
                             ((int_of_char msg_size_str.[3]) lsl 24 )) in
          let download_request = String.make msg_size '0' in
          ignore (Unix.recv dlman_sock download_request 0 msg_size []);
          ignore (add_download download_request)
        done
      with Unix.Unix_error (error, cmd, loc) ->
        print_string "Unable to receive request.\n"
  with Unix.Unix_error (err, cmd, loc) ->
    print_string ("Unable to launch daemon! Generated error in " ^ cmd ^ "\n");
    Printf.eprintf "Error code: %s\n" (Unix.error_message err);
    Sys.remove "/tmp/dlman_socket"
(* That loop will run forever, at least right now. I should create a method
   to determine when it should be cancelled. *)
    (* This does not remove the fifo currently. That needs to be resolved. *)

let _ =
  Curl.global_init Curl.CURLINIT_GLOBALALL;
  let dl_dir = "/Users/ben/Downloads" in (* This needs to become loaded from a
  file. *)
  if (Array.length Sys.argv) > 2 then
    if (String.compare (Sys.argv.(1)) "--add") = 0 then
      if (Sys.file_exists ("/tmp/dlman_socket")) then
        for i=2 to ((Array.length Sys.argv) - 1) do
          send_job (Sys.argv.(i))
        done
      else
        print_string "There does not appear to be a dlman daemon running!\n"
    else
      print_string "Incorrect invocation of dlman.\n"
  else
    if (Array.length Sys.argv) = 2 then
      if (String.compare Sys.argv.(1) "--start-daemon" = 0) then
        read_loop dl_dir
      else
        let new_thread = add_download Sys.argv.(1) in
        print_string "Waiting on download . . .\n";
        Thread.join new_thread;
        print_string "Finished waiting.\n"
    else
      print_string "Incorrect invocation of dlman.\n"
