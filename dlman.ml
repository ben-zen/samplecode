(* Let's make a DL manager with a web interface! *)
(* Copyright (C) 2013 Ben Lewis <benjf5+bitbucket@gmail.com>,
   <benjf5+github@gmail.com> *)
(* Under the LGPLv3 license. *)

(* Uses OcURL and libcurl. *)

(* Uses Unix, Str, String, Buffer, Digest (for now), Curl, and Thread. *)

(* Bytecode compilation command:
   ocamlc -thread -o dlman unix.cma curl.cma str.cma threads.cma dlman.ml
   Native code compilation command:
   ocamlopt -thread -o dlman unix.cmxa curl.cmxa str.cmxa threads.cmxa dlman.ml
*)

(* This ... does not seem like the best solution, but for now it's what I'm
   going with.  The goal is to provide a way to generate a package of updates
   for the server once it exists, but I'm not sure how to best present this
   information. *)

type status_update =
  Progress of Digest.t * float
| Completion of Digest.t

let print_usage () =
  print_string
    ("dlman: A download manager with remote capabilities.\n"
     ^ "Options:\n"
     ^ "--add FILE_ADDRESS: Downloads FILE_ADDRESS to the download folder.\n"
     ^ "--start-daemon: Launches daemon to download to default download "
     ^ "folder.\n" 
     ^ "--kill-daemon: Stops running daemon.\n")

let determine_progress dl_mon file_digest connection =
  let total_complete = Curl.get_sizedownload connection
  and total_size = Curl.get_contentlengthdownload connection in
  let current_complete =  ((total_complete /. total_size ) *. 100.0) in
  ignore (Event.poll (Event.send dl_mon (Progress(file_digest,
                                                  current_complete))))

let write_data fp data_buffer data_line =
  let data_length = String.length data_line in 
  Buffer.add_string data_buffer data_line;
  Buffer.output_buffer fp data_buffer;
  Buffer.reset data_buffer;
  data_length

let write_wrapper file_digest dl_mon connection fp data_buffer data_line =
  determine_progress dl_mon file_digest connection;
  write_data fp data_buffer data_line

let dl_file dl_mon file_location =
  try 
    let location_list = Str.split (Str.regexp "/") file_location in
    let file_name = List.hd (List.rev location_list) in
    let file_p = open_out_bin (file_name) in
    let file_digest = Digest.string file_location in
    let incoming_data = Buffer.create 4096
    (* using a simple buffer at the moment. This will become configurable. *)
    and connection = Curl.init () in
    Curl.set_writefunction connection (write_wrapper file_digest dl_mon
                                         connection file_p incoming_data);
    Curl.set_followlocation connection true;
    Curl.set_url connection file_location;
    Curl.perform connection;
    Curl.cleanup connection;
    close_out file_p;
    Event.sync (Event.send dl_mon (Completion(file_digest)));
    0
  with Curl.CurlException (_, _, s) -> print_string s; 1

let add_download dl_mon file_location =
  (Thread.create (dl_file dl_mon) file_location)

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
    print_string "Job sent to daemon.\n";
    Unix.close dlman_add_sock;
    Sys.remove "/tmp/dlman_add_socket"
  with Unix.Unix_error (err, cmd, loc) ->
    (print_string ("Failed to send job to daemon. Generated error in " ^ cmd ^
                      ".\n");
     Printf.eprintf "Error message: %s.\n" (Unix.error_message err);     
     Sys.remove "/tmp/dlman_add_socket")

let status_monitor dl_mon =
  while true do
    try
      let status_message = Event.sync (Event.receive dl_mon) in
      match status_message with
        Progress (digest, progress) ->
          print_string ( "{ \"digest\": \"" ^ (Digest.to_hex digest)
                         ^ "\"; \"progress\": " ^ (string_of_float progress)
                         ^ " }" )
      | Completion (digest) ->
        print_string ( "{ \"digest\": \"" ^ (Digest.to_hex digest)
                       ^ "\"; \"download_complete\": true }" )
    with except -> ()
  done
      
let read_loop dl_mon =
  try
    let pid_file = open_out "/tmp/dlman.pid" in
    output_string pid_file ((string_of_int (Unix.getpid ()) ^ "\n"));
    close_out pid_file;
    let dlman_sock = Unix.socket Unix.PF_UNIX Unix.SOCK_DGRAM 0 in
    Unix.bind dlman_sock (Unix.ADDR_UNIX("/tmp/dlman_socket"));
    while true do
      try
          let msg_size_str = String.create 4 in
          ignore (Unix.recv dlman_sock msg_size_str 0 4 []);
          let msg_size = ((int_of_char msg_size_str.[0]) +
                             ((int_of_char msg_size_str.[1]) lsl 8) +
                             ((int_of_char msg_size_str.[2]) lsl 16) + 
                             ((int_of_char msg_size_str.[3]) lsl 24 )) in
          let download_request = String.make msg_size '0' in
          ignore (Unix.recv dlman_sock download_request 0 msg_size []);
          ignore (add_download dl_mon download_request)
      with Unix.Unix_error (error, cmd, loc) ->
        print_string ("Unable to receive request.  Error generated in " ^ cmd ^
                         ".\n");
        Printf.eprintf "Error code: %s.\n" (Unix.error_message error);
    done
  with Unix.Unix_error (err, cmd, loc) ->
    print_string ("Unable to launch daemon! Generated error in " ^ cmd ^ ".\n");
    Printf.eprintf "Error code: %s.\n" (Unix.error_message err);
    Sys.remove "/tmp/dlman_socket"

(* The current state of signals will simply stop communications, does not
   actually close downloads or clean up much.  That is going to be a future
   development, possibly using a list of active threads.  If that is the route
   taken, the loop component of this will probably become a tail-recursive
   operation. *)

(* serve_page is currently a dummy method that will be improved, because
   really, who's gonna want to use *that* method right now? *)
      
let serve_page client_socket =
  let pagedata =
    "HTTP/1.1 200 OK \r\n"
    (*^ "Date: " *)
    ^ "Content-Length: 37\r\n"
    ^ "Content-Type: text/html\r\n\r\n"
    ^ "<html>\n<body> Testing. </body></html>"
  in
  ignore (Unix.send client_socket pagedata 0 (String.length pagedata) [])
      
let server port =
  try
    let host = (Unix.gethostbyname (Unix.gethostname ())).Unix.h_addr_list.(0) in
    let addr = Unix.ADDR_INET(Unix.inet_addr_loopback, port) in
    let server_sock = Unix.socket Unix.PF_INET Unix.SOCK_STREAM 0 in
    Unix.bind server_sock addr;
    Unix.listen server_sock 5;
    while true do
      let (client_sock, client_addr) = Unix.accept server_sock in
      ignore (Thread.create serve_page client_sock);
    done
  with Unix.Unix_error (err, cmd, loc) ->
    print_string ("Failed to start server.  Generated error in " ^ cmd ^ ".\n");
    Printf.eprintf "Error code: %s.\n" (Unix.error_message err)

let initialize () =
  Curl.global_init Curl.CURLINIT_GLOBALALL;
  try
    Sys.set_signal Sys.sigusr1
      (Sys.Signal_handle(fun x ->
        try
          Sys.remove "/tmp/dlman_socket";
          Sys.remove "/tmp/dlman.pid";
          print_string "Stopping daemon.\n";
          ignore (exit 0)
        with Sys_error (s) ->
          print_string ("Error in shutdown: " ^ s ^ ". Stopping daemon.\n");
          ignore (exit 1)))
  with Invalid_argument (x) ->
    print_string "Initializing signal failed.\n"

let start_daemon () =
  initialize ();
  let dl_mon = (Event.new_channel ()) in
  ignore (Thread.create status_monitor dl_mon);
  ignore (Thread.create server 8080);
  read_loop dl_mon
      
let kill_daemon () =
  try
    let pid_find = open_in "/tmp/dlman.pid" in
    let pid_daemon = (int_of_string (input_line pid_find)) in
    Unix.kill pid_daemon Sys.sigusr1
  with Sys_error (s) ->
    print_string "No running daemon instance.\n"

let _ =
  let dl_dir = "/Users/ben/Downloads/" in
  Unix.chdir dl_dir;
  (* This needs to become loaded from a file. *)
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
        let stat = Unix.fork () in match stat with
          | 0 -> start_daemon ()
          | _ -> print_string "Launching daemon.\n"
      else if (String.compare Sys.argv.(1) "--kill-daemon" = 0) then
        kill_daemon ()
      else
        (*initialize ();*)
        let dl_mon = Event.new_channel () in
        ignore (Thread.create status_monitor dl_mon);
        let new_thread = add_download dl_mon Sys.argv.(1) in
        Thread.join new_thread;
      else
        print_usage ()

(* This needs to be more robust; there will need to be a better string-matching
   procedure for it to really work without doing unexpected and terrible
   things. *)
