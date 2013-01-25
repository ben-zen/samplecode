(* Let's make a DL manager with a web interface! *)
(* Uses OcURL and libcurl. *)

(* Uses Unix, Str, String, Buffer, Digest (for now), Curl, and Thread. *)

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

(* Needs functions to produce a webpage, now that it downloads files. *)

(* let add_download file_location =
  Thread.create (dl_file file_location) *)
    (* This function will need to become slightly more complicated, and it will
  need a function around dl_file, since we'll be handling sending output to a
  webpage (if launched), etc. *)

let acquire_lock download_dir =
  Unix.chdir download_dir;
  if not (Sys.file_exists "./.dlman.daemon.pid")
  then
    try
      let loc_file = open_out "./.dlman.daemon.pid" in
      let cpid = Unix.getpid () in
      let current_pid = string_of_int cpid in
      output_string loc_file current_pid;
      close_out loc_file;
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

(* Necessary functionality includes a way to add a download to a
 * currently-running dlman instance, and a web view. Probably also a list of
 * active downloads, and later a list of completed downloads.
 *)

let send_job download_dir file_location =
  if (Sys.file_exists (download_dir ^ "/.dlman.daemon.pid"))
  then
      let outpipe = Unix.openfile (download_dir ^ "/.dlman.add")
        [Unix.O_WRONLY] 0o640 in
      ignore(Unix.write outpipe file_location (String.length file_location) 0);
      Unix.close outpipe;
  else
    ignore(print_string "No dlman daemon running in download directory specified!")
      
let read_loop download_dir =
  try
    Unix.mkfifo (download_dir ^ "/.dlman.add") 0o640;
    let inpipe = Unix.openfile (download_dir ^ "/.dlman.add") [Unix.O_RDONLY]
      0o640 in
    let inchannel = Unix.in_channel_of_descr inpipe in
    let dest_buffer = Buffer.create 20 in
    while true do
      Buffer.add_string dest_buffer (input_line inchannel);
      dl_file (Buffer.contents dest_buffer); (* This will be replaced by a
                                                thread-based add_download method once that is possible. *)
      Buffer.reset dest_buffer;
    done
  with Unix.Unix_error (err, cmd, loc) ->
    print_string "Unable to launch daemon!\n"
(* That loop will run forever, at least right now. I should create a method
   to determine when it should be cancelled. *)
    (* This does not remove the fifo currently. That needs to be resolved. *)

let launch () =
  let dl_dir = "/home/ben/Downloads" in
  if (Array.length Sys.argv) > 1 then
    if (String.compare (Sys.argv.(0)) "--add") = 0 then
      if (Sys.file_exists (dl_dir ^ "/.dlman.daemon.pid")) then
        for i=1 to ((Array.length Sys.argv) - 1) do
          send_job dl_dir (Sys.argv.(i) ^ "\n")
        done
      else
        print_string "There does not appear to be a dlman daemon running!\n"
    else
      print_string "Incorrect invocation of dlman.\n"
  else
    if (String.compare (Sys.argv.(0)) "--start-daemon") = 0 then
      let daemon_lock = acquire_lock dl_dir in
      if daemon_lock = 0 then
        read_loop dl_dir
      else
        ()
    else
      ()
(*
  print_string "Incorrect invocation of dlman.\n" *)
    
