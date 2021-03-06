(* Let's make a DL manager with a web interface! *)
(* Copyright (C) 2013 Ben Lewis <benjf5+bitbucket@gmail.com>,
   <benjf5+github@gmail.com> *)
(* Under the LGPLv3 license. *)

(* Uses OcURL and libcurl. *)

(* Uses Unix, Str, String, Buffer, Digest (for now), Curl, and Thread. *)

(* Bytecode compilation command:
   ocamlc -thread -I +curl -o dlman
   unix.cma curl.cma str.cma threads.cma dlman.ml
   Native code compilation command:
   ocamlopt -thread -I +curl -o dlman
   unix.cmxa curl.cmxa str.cmxa threads.cmxa dlman.ml
*)

type download_list = {
  mut : Mutex.t ;
  downloads : ( Digest.t , Digest.t * string * string * float ) Hashtbl.t
}

exception ConflictingDigest of Digest.t

type status_update =
  Start of Digest.t * string * string
| Progress of Digest.t * float
| Completion of Digest.t

type http_data =
  HTML of string
| JSON of string

let print_usage () =
  print_string
    ("dlman: A download manager with remote capabilities.\n"
     ^ "Options:\n"
     ^ "--add FILE_ADDRESS: Downloads FILE_ADDRESS to the download folder.\n"
     ^ "--start-daemon: Launches daemon to download to default download "
     ^ "folder.\n" 
     ^ "--kill-daemon: Stops running daemon.\n")

let download_list_create () = {
  mut = Mutex.create () ;
  downloads = Hashtbl.create ~random:true 30
}

let download_list_add dl_list file_digest file_location file_name =
  try
    Mutex.lock dl_list.mut;
    if (Hashtbl.mem dl_list.downloads file_digest)
    then raise (ConflictingDigest file_digest)
    else
      Hashtbl.add dl_list.downloads file_digest
        (file_digest, file_location, file_name, 0.0);
    Mutex.unlock dl_list.mut
  with ConflictingDigest(dat) ->
    Mutex.unlock dl_list.mut;
    print_string ("Thread "
                  ^ (string_of_int (Thread.id (Thread.self ())))
                  ^ ": Unable to add download; previously existing digest: "
                  ^ Digest.to_hex dat ^ ".\n");
    raise (ConflictingDigest dat)

let download_list_remove dl_list file_digest =
  Mutex.lock dl_list.mut;
  Hashtbl.remove dl_list.downloads file_digest;
  Mutex.unlock dl_list.mut

let download_list_find dl_list file_digest =
  Mutex.lock dl_list.mut;
  try
    let found = Hashtbl.find dl_list.downloads file_digest in
    Mutex.unlock dl_list.mut;
    Some found
  with exn ->
    Mutex.unlock dl_list.mut;
    None

let download_list_update dl_list file_digest completion =
  Mutex.lock dl_list.mut;
  try
    let (_, file_location, file_name, _) =
      Hashtbl.find dl_list.downloads file_digest in
    Hashtbl.replace dl_list.downloads file_digest
      (file_digest, file_location, file_name, completion);
    Mutex.unlock dl_list.mut;
  with exn ->
    Mutex.unlock dl_list.mut

let determine_progress active_list dl_mon file_digest connection =
  let total_complete = Curl.get_sizedownload connection
  and total_size = Curl.get_contentlengthdownload connection in
  let current_complete =  ((total_complete /. total_size ) *. 100.0) in
  ignore (Event.poll (Event.send dl_mon (Progress(file_digest,
                                                  current_complete))));
  download_list_update active_list file_digest current_complete

let write_data fp data_buffer data_line =
  let data_length = String.length data_line in 
  Buffer.add_string data_buffer data_line;
  Buffer.output_buffer fp data_buffer;
  Buffer.reset data_buffer;
  data_length

let write_wrapper active_list file_digest dl_mon connection fp data_buffer
    data_line =
  determine_progress active_list dl_mon file_digest connection;
  write_data fp data_buffer data_line

let dl_file dl_lists dl_mon file_location =
  let (active_list, complete_list) = dl_lists in
  try
    let file_digest = Digest.string file_location in
    let location_list = Str.split (Str.regexp "/") file_location in
    let file_name = List.hd (List.rev location_list) in
    download_list_add active_list file_digest file_location file_name;
    let file_p = open_out_bin (file_name) in
    let incoming_data = Buffer.create 4096
    (* using a simple buffer at the moment. This will become configurable. *)
    and connection = Curl.init () in
    Event.sync (Event.send dl_mon (Start(file_digest,file_location,file_name)));
    Curl.set_writefunction connection (write_wrapper active_list file_digest
                                         dl_mon connection file_p
                                         incoming_data);
    Curl.set_followlocation connection true;
    Curl.set_url connection file_location;
    Curl.perform connection;
    Curl.cleanup connection;
    close_out file_p;
    ignore (download_list_remove active_list file_digest);
    download_list_add complete_list file_digest file_location file_name;
    Event.sync (Event.send dl_mon (Completion(file_digest)));
    0
  with
    Curl.CurlException (_, _, s) -> print_string s; 1
  | ConflictingDigest (d) -> print_string
    ( "Trying to download the file at " ^ file_location
    ^ " generated a conflicting MD5 hash of " ^ (Digest.to_hex d)
    ^ ", which is already present for another download.  "
    ^ "This may generate a filename conflict.\n" ); 1

let add_download dl_lists dl_mon file_location =
  (Thread.create (dl_file dl_lists dl_mon) file_location)

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

let status_monitor active_downloads completed_downloads dl_mon =
  while true do
    try
      let status_message = Event.sync (Event.receive dl_mon) in
      match status_message with
        Start (digest, location, filename) ->
          print_string ("{ \"digest\": \"" ^ (Digest.to_hex digest)
                        ^ "; \"download_complete\": false; "
                        ^"\"progress\": 0.0 }\n")
      | Progress (digest, progress) ->
        print_string ( "{ \"digest\": \"" ^ (Digest.to_hex digest)
                         ^ "\"; \"progress\": " ^ (string_of_float progress)
                         ^ " }\n" )
      | Completion (digest) ->

        print_string ( "{ \"digest\": \"" ^ (Digest.to_hex digest)
                       ^ "\"; \"download_complete\": true }\n" )
    with exn -> print_string (Printexc.to_string exn)
  done
      
let read_loop active_downloads completed_downloads dl_mon =
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
          ignore (add_download (active_downloads, completed_downloads)
                    dl_mon download_request)
      with Unix.Unix_error (error, cmd, loc) ->
        print_string ("Unable to receive request.  Error generated in " ^ cmd ^
                         ".\n");
        Printf.eprintf "Error code: %s.\n" (Unix.error_message error);
    done
  with Unix.Unix_error (err, cmd, loc) ->
    print_string ("Unable to launch daemon! Generated error in " ^ cmd ^ ".\n");
    Printf.eprintf "Error code: %s.\n" (Unix.error_message err);
    Sys.remove "/tmp/dlman_socket"
  | Not_found -> Printexc.print_backtrace stdout;
    Unix.kill (Unix.getpid ()) Sys.sigusr1
  | exn -> print_string (Printexc.to_string exn)

(* The current state of signals will simply stop communications, does not
   actually close downloads or clean up much.  That is going to be a future
   development, possibly using a list of active threads.  If that is the route
   taken, the loop component of this will probably become a tail-recursive
   operation. *)


(* HTTP responses need to be modified to reply to a particular incoming
   message.  The current system is to simply throw output at the asking system,
   but that's not what's called for, especially as more is added. *)

let format_HTTP_response data =
  let content_type = match data with
      HTML (_) -> "text/html"
    | JSON (_) -> "text/json"
  in
  let content_length = match data with
      HTML (s) -> string_of_int (String.length s)
    | JSON (s) -> string_of_int (String.length s)
  in
  let content_matter = match data with
      HTML (s) -> s
    | JSON (s) -> s
  in
  "HTTP/1.1 200 OK\r\n"
  ^ "Content-Length: " ^ content_length ^ "\r\n"
  ^ "Content-Type: " ^ content_type ^ "\r\n\r\n"
  ^ content_matter

let generate_download_list_html dl_list completed =
  Mutex.lock dl_list.mut;
  let created_html = Hashtbl.fold
    (fun _ (dig, src, loc, per) html ->
      (html
       ^ "<div class=\"download\" id=\"" ^ Digest.to_hex dig ^ "\">\n"
       ^ "<p class=\"filename\">"
       ^ loc ^ "</p>\n"
       ^ ( if not completed then "<progress max=\"100\" value=\""
           ^ (string_of_int (int_of_float per)) ^ "></progress>\n"
         else "" )
       ^ "</div>\n"
      )
    ) dl_list.downloads "" in
  Mutex.unlock dl_list.mut;
  created_html

let construct_main_page dl_lists =
  let (active_downloads, completed_downloads) = dl_lists in
  "<!doctype html>\n<html lang=\"en\">\n<head>\n<title>dlman</title>\n"
  ^ "<meta charset=\"utf-8\" />\n</head>\n<body>\n<h1>dlman</h1>\n"
  ^ "<div id=\"active_downloads\">\n<h2>Active Downloads</h2>\n"
  ^ (generate_download_list_html active_downloads false)
  ^ "</div>\n<div id=\"completed_downloads\">\n<h2>Completed Downloads</h2>\n"
  ^ (generate_download_list_html completed_downloads true)
  ^ "</div>\n</body>\n</html>\n"

(* serve_page is currently a dummy method that will be improved, because
   really, who's gonna want to use *that* method right now? *)

let serve_client dl_lists client_socket client_channel =
  let pagedata = format_HTTP_response
    (HTML(construct_main_page dl_lists)) in 
  ignore (Unix.send client_socket pagedata 0 (String.length pagedata) []);
  try
    while true do
      ignore (Event.sync (Event.receive client_channel))
    (* This will be replaced with sending updates to the client once I've found
       a way to make that happen. *)
    done
  with Unix.Unix_error (err, cmd, loc) -> ()
    (* Encountered from EPIPE, which gets sent instead of SIGPIPE because
       SIGPIPE is ignored re. initialization. *)

let server dl_lists port =
  try
    let serving_pages = [] in
    let host = (Unix.gethostbyname (Unix.gethostname ())).Unix.h_addr_list.(0) in
    let addr = Unix.ADDR_INET(Unix.inet_addr_loopback, port) in
    let server_sock = Unix.socket Unix.PF_INET Unix.SOCK_STREAM 0 in
    Unix.bind server_sock addr;
    Unix.listen server_sock 5;
    while true do
      let (client_sock, client_addr) = Unix.accept server_sock in
      let client_channel = Event.new_channel () in
      let client_thread = (Thread.create (serve_client dl_lists client_sock)
                             client_channel) in
      ignore (serving_pages = (client_thread, client_channel) :: serving_pages)
    (* Still need to find a proper next step to take on this one; that list
       needs to be updated regularly against closed threads; updates should be
       dispatched to each thread to be sent to the pages they're serving, and
       when the page is closed, the thread should be closed and its element
       removed from the list.  Which is where it gets tricky.  Considering
       adding a "connection_status" type and channel heading in the other
       direction. *)
    done
  with Unix.Unix_error (err, cmd, loc) ->
    print_string ("Failed to start server.  Generated error in " ^ cmd ^ ".\n");
    Printf.eprintf "Error code: %s.\n" (Unix.error_message err)
      
let initialize () =
  Curl.global_init Curl.CURLINIT_GLOBALALL;
  try
    Sys.set_signal Sys.sigpipe Sys.Signal_ignore;
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
  let active_downloads = download_list_create () in
  let completed_downloads = download_list_create() in
  let dl_lists = (active_downloads, completed_downloads) in
  let dl_mon = (Event.new_channel ()) in
  let mon_thread = Thread.create (status_monitor dl_lists) dl_mon in
  (*let server_thread = Thread.create (server dl_lists) 8080 in*)
  print_string ("Monitor thread: " ^ (string_of_int (Thread.id mon_thread)));
  (*print_string ("Server thread: " ^ (string_of_int (Thread.id server_thread)));*)
  read_loop active_downloads completed_downloads dl_mon
      
let kill_daemon () =
  try
    let pid_find = open_in "/tmp/dlman.pid" in
    let pid_daemon = (int_of_string (input_line pid_find)) in
    Unix.kill pid_daemon Sys.sigusr1
  with Sys_error (s) ->
    print_string "No running daemon instance.\n"

let _ =
  try
    let home_dir = Sys.getenv "HOME" in
    let config_stream = open_in (home_dir ^ "/.dlman_conf") in
    let dl_dir = input_line (config_stream) in
    Unix.chdir dl_dir;
    close_in config_stream; (* For the time being *)
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
      (*      else
        (*initialize ();*)
              let dl_mon = Event.new_channel () in
              ignore (Thread.create status_monitor dl_mon);
              let new_thread = add_download dl_lists dl_mon Sys.argv.(1) in
              Thread.join new_thread; *)
        else if (String.compare Sys.argv.(1) "--halp" = 0) then
          print_string "dlman: You can haz downloads!\n"
        else
          print_usage ()
        else
          print_usage ()
  with Sys_error e -> prerr_endline e
  | exn -> print_string ("Failed to launch daemon; exception " ^
  Printexc.to_string exn ^ ".\n")
(* This needs to be more robust; there will need to be a better string-matching
   procedure for it to really work without doing unexpected and terrible
   things. *)
