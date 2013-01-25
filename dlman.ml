(* Let's make a DL manager with a web interface! *)
(* Uses OcURL and libcurl. *)

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
  


let acquire_lock download_dir =
  Unix.chdir download_dir;
  if not (Sys.file_exists "./.dlman.daemon.pid")
  then
    let loc_file = open_out "./.dlman.daemon.pid" in
    let cpid = Unix.getpid () in
    let current_pid = string_of_int cpid in
    output_string loc_file current_pid;
    close_out loc_file;
  else
    print_string ( "The lock file, .dlman.daemon.pid, exists in the download " ^
      "directory specified; if there is no currently-running dlman instance, " ^
      "erase the lock file and start dlman again." );
    exit 1
      (* In this case, the daemon will not start.  No other action need be
       * performed. *)    
  
