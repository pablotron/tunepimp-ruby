/************************************************************************/
/* Copyright (c) 2004 Paul Duncan                                       */
/*                                                                      */
/* Permission is hereby granted, free of charge, to any person          */
/* obtaining a copy of this software and associated documentation files */
/* (the "Software"), to deal in the Software without restriction,       */
/* including without limitation the rights to use, copy, modify, merge, */
/* publish, distribute, sublicense, and/or sell copies of the Software, */
/* and to permit persons to whom the Software is furnished to do so,    */
/* subject to the following conditions:                                 */
/*                                                                      */
/* The above copyright notice and this permission notice shall be       */
/* included in all copies of the Software, its documentation and        */
/* marketing & publicity materials, and acknowledgment shall be given   */
/* in the documentation, materials and software packages that this      */
/* Software was used.                                                   */
/*                                                                      */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,      */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF   */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND                */
/* NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY     */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE    */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.               */
/************************************************************************/

#include <tunepimp/tp_c.h>
#include <ruby.h>

#define VERSION "0.1.0"
#define UNUSED(a) ((void) (a))

static VALUE mTP,
             cTP,
             cTr,
             mPrio,
             mErr,
             mCB,
             mStat,
             cMD,
             mRT,
             eException;


static void tp_md_free(void *md) {
  if (md) {
    md_Delete(*(metadata_t**) md);
    free(md);
  }
}

/*********************************************************************/
/* TunePimp::TunePimp methods                                        */
/*********************************************************************/
static void tp_tp_free(void *tp) {
  if (tp) {
    tp_Delete(* ((tunepimp_t**) tp));
    free(tp);
  }
}

/*
 * Create a new TunePimp::TunePimp object.
 *
 * Examples:
 *   # create new tunepimp object
 *   tp = TunePimp::TunePimp.new('PimpApp', 'PimpApp 1.0')
 *
 *   # create new tunepimp object, start threads
 *   tp = TunePimp::TunePimp.new('PimpApp', 'PimpApp 1.0', true)
 *
 */
VALUE tp_tp_new(int argc, VALUE *argv, VALUE klass) {
  tunepimp_t *tp;
  VALUE self;

  if ((tp = malloc(sizeof(tunepimp_t))) == NULL)
    rb_raise(eException, "Couldn't allocate memory for tunepimp_t");

  switch (argc) {
    case 2:
      tp = tp_New(RSTRING(argv[0])->ptr, RSTRING(argv[1])->ptr);
      break;
    case 3:
      tp = tp_NewWithArgs(RSTRING(argv[0])->ptr,
                          RSTRING(argv[1])->ptr, 
                          !(argv[2] == Qnil || argv[2] == Qfalse));
      break;
    default:
      rb_raise(rb_eArgError, "invalid argument count (not 2 or 3)");
  }

  self = Data_Wrap_Struct(klass, 0, tp_tp_free, tp);
  rb_obj_call_init(self, 0, NULL);

  return self;
}

/*
 * Constructor for TunePimp::TunePimp object.
 *
 * This method is currently empty.  You should never call this method
 * directly unless you're instantiating a derived class (ie, you know
 * what you're doing).
 *
 */
static VALUE tp_tp_init(VALUE self) {
  return self;
}

/*
 * Get the major. minor, and revision version of the TunePimp library.
 *
 * Example:
 *   ary = tp.version
 *   puts 'TunePimp version: ' << ary.join('.')
 *
 */
static VALUE tp_tp_ver(VALUE self) {
  tunepimp_t *tp;
  int i, vals[3];
  VALUE ret;

  Data_Get_Struct(self, tunepimp_t, tp);
  ret = rb_ary_new();

  tp_GetVersion(*tp, &vals[0], &vals[1], &vals[2]);
  for (i = 0; i < 3; i++)
    rb_ary_push(ret, INT2FIX(vals[i]));
  
  return ret;
}

/*
 * Set the username and password for this TunePimp::TunePimp object.
 *
 * Example:
 *   tp.set_user_info('pimpuser', 'p1mpU534')
 *
 */
static VALUE tp_tp_set_user_info(VALUE self, VALUE user, VALUE pass) {
  tunepimp_t *tp;
  Data_Get_Struct(self, tunepimp_t, tp);
  tp_SetUserInfo(*tp, RSTRING(user)->ptr, RSTRING(pass)->ptr);
  return Qnil;
}

/*
 * Get the username and password for this TunePimp::TunePimp object.
 *
 * Aliases:
 *   TunePimp::TunePimp#user_info
 *   TunePimp::TunePimp#get_user_info
 *
 * Example:
 *   user, pass = tp.user_info
 *
 */
static VALUE tp_tp_get_user_info(VALUE self) {
  tunepimp_t *tp;
  char user[1024], pass[1024];
  VALUE ret;

  Data_Get_Struct(self, tunepimp_t, tp);
  tp_GetUserInfo(*tp, user, 1024, pass, 1024);

  ret = rb_ary_new();
  rb_ary_push(ret, rb_str_new2(user));
  rb_ary_push(ret, rb_str_new2(pass));

  return ret;
}

/*
 * Use the UTF-8 for this TunePimp::TunePimp object?
 *
 * Note: Defaults to false, or ISO-8859-1 (latin1).
 *
 * Example:
 *   tp.use_utf8 = true
 *
 */
static VALUE tp_tp_set_use_utf8(VALUE self, VALUE utf8) {
  tunepimp_t *tp;
  Data_Get_Struct(self, tunepimp_t, tp);
  tp_SetUseUTF8(*tp, !(utf8 == Qfalse || utf8 == Qnil));
  return Qnil;
}

/*
 * Is this TunePimp::TunePimp object using UTF-8?
 *
 * Note: False means ISO-8859-1 (latin1).
 *
 * Example:
 *   puts 'Character set: ' << (tp.use_utf8 ? 'UTF-8' : 'ISO-8859-1')
 *
 */
static VALUE tp_tp_get_use_utf8(VALUE self) {
  tunepimp_t *tp;
  Data_Get_Struct(self, tunepimp_t, tp);
  return tp_GetUseUTF8(*tp) ? Qtrue : Qfalse;
}

/*
 * Set the server and port for this TunePimp::TunePimp object.
 *
 * Note: defaults to www.musicbrainz.org, port 80.
 *
 * Example:
 *   tp.set_server('www.musicbrainz.org', 80)
 *
 */
static VALUE tp_tp_set_server(VALUE self, VALUE host, VALUE port) {
  tunepimp_t *tp;
  Data_Get_Struct(self, tunepimp_t, tp);
  tp_SetServer(*tp, RSTRING(host)->ptr, NUM2INT(port));
  return Qnil;
}

/*
 * Get the server and port for this TunePimp::TunePimp object.
 *
 * Aliases:
 *   TunePimp::TunePimp#server
 *   TunePimp::TunePimp#get_server
 * 
 * Example:
 *   host, port = tp.server
 *
 */
static VALUE tp_tp_get_server(VALUE self) {
  tunepimp_t *tp;
  char host[1024];
  short port;
  VALUE ret;

  Data_Get_Struct(self, tunepimp_t, tp);
  tp_GetServer(*tp, host, 1024, &port);

  ret = rb_ary_new();
  rb_ary_push(ret, rb_str_new2(host));
  rb_ary_push(ret, INT2FIX(port));
  
  return ret;
}

/*
 * Set the proxy host and port for this TunePimp::TunePimp object.
 *
 * Note: Defaults to off.  Set to host '' or nil to disable.
 *
 * Examples:
 *   # set the proxy server
 *   tp.set_proxy('proxy.home.pablotron.org', 8080)
 *
 *   # disable the proxy server
 *   tp.set_proxy(nil)
 */
static VALUE tp_tp_set_proxy(int argc, VALUE *argv, VALUE self) {
  tunepimp_t *tp;
  Data_Get_Struct(self, tunepimp_t, tp);

  switch (argc) {
    case 1:
      if (argv[0] != Qnil)
        rb_raise(rb_eArgError, "missing proxy port");
      else
        tp_SetProxy(*tp, "", 0);
      break;
    case 2:
      if (argv[0] != Qnil)
        tp_SetProxy(*tp, RSTRING(argv[0])->ptr, NUM2INT(argv[1]));
      else
        tp_SetProxy(*tp, "", 0);

      break;
    default:
      rb_raise(rb_eArgError, "invalid argument count (not 1 or 2)");
  }

  return Qnil;
}

/*
 * Get the proxy host and port for this TunePimp::TunePimp object.
 *
 * Note: Returns nil if there is no proxy set.
 *
 * Aliases:
 *   TunePimp::TunePimp#proxy
 *   TunePimp::TunePimp#get_proxy
 * 
 * Example:
 *   proxy_host, proxy_port = tp.proxy
 *
 */
static VALUE tp_tp_get_proxy(VALUE self) {
  tunepimp_t *tp;
  char host[1024];
  short port;
  VALUE ret;

  Data_Get_Struct(self, tunepimp_t, tp);
  tp_GetProxy(*tp, host, 1024, &port);

  ret = Qnil;
  if (strlen(host) > 0) {
    ret = rb_ary_new();
    rb_ary_push(ret, rb_str_new2(host));
    rb_ary_push(ret, INT2FIX(port));
  }
  
  return ret;
}

/*
 * Number of extensions supported by this TunePimp::TunePimp object.
 *
 * Aliases:
 *   TunePimp::TunePimp#num_supported_extensions
 *   TunePimp::TunePimp#get_num_supported_extensions
 *
 * Example:
 *   puts "Supports #{tp.num_supported_extensions} extensions."
 *
 */
static VALUE tp_tp_num_exts(VALUE self) {
  tunepimp_t *tp;
  Data_Get_Struct(self, tunepimp_t, tp);
  return INT2FIX(tp_GetNumSupportedExtensions(*tp));
}

/*
 * Get extensions supported by this TunePimp::TunePimp object.
 *
 * Aliases:
 *   TunePimp::TunePimp#supported_extensions
 *   TunePimp::TunePimp#get_supported_extensions
 *
 * Example:
 *   puts "Extensions: \"#{tp.supported_extensions.join('", "')}\"."
 *
 */
static VALUE tp_tp_exts(VALUE self) {
  tunepimp_t *tp;
  char **exts;
  int i, num;
  VALUE ret;
  
  Data_Get_Struct(self, tunepimp_t, tp);

  num = tp_GetNumSupportedExtensions(*tp);
  if ((exts = malloc(sizeof(char*) * num)) == NULL)
    rb_raise(eException, "Couldn't allocate %d bytes for char**",
             sizeof(char*) * num);
  for (i = 0; i < num; i++)
    if ((exts[i] = malloc(TP_EXTENSION_LEN)) == NULL)
      rb_raise(eException, "Couldn't allocate %d bytes for char*", TP_EXTENSION_LEN);
  
  tp_GetSupportedExtensions(*tp, exts);

  ret = rb_ary_new();
  for (i = 0; i < num; i++) {
    rb_ary_push(ret, rb_str_new2(exts[i]));
    free(exts[i]);
  }
  free(exts);

  return ret;
}

/*
 * Set the analyzer thread priority for this TunePimp::TunePimp object.
 *
 * Valid values are as follows:
 *   TunePimp::ThreadPriority::Idle
 *   TunePimp::ThreadPriority::Lowest
 *   TunePimp::ThreadPriority::Low
 *   TunePimp::ThreadPriority::Normal
 *   TunePimp::ThreadPriority::High
 *   TunePimp::ThreadPriority::Higher
 *   TunePimp::ThreadPriority::TimeCritical
 *
 * Example:
 *   tp.analyzer_priority = TunePimp::ThreadPriority::High
 *
 */
static VALUE tp_tp_set_analyzer_prio(VALUE self, VALUE prio) {
  tunepimp_t *tp;
  int p;

  p = NUM2INT(prio);
  if (p < eIdle || p > eTimeCritical)
    rb_raise(eException, "Thread Priority out of range");

  Data_Get_Struct(self, tunepimp_t, tp);
  tp_SetAnalyzerPriority(*tp, p);
  return prio;
}

/*
 * Get the analyzer thread priority for this TunePimp::TunePimp object.
 *
 * Example:
 *   if tp.analyzer_priority == TunePimp::ThreadPriority::Normal
 *     puts "Analyzer Priority is normal"
 *   end
 *
 */
static VALUE tp_tp_analyzer_prio(VALUE self) {
  tunepimp_t *tp;
  Data_Get_Struct(self, tunepimp_t, tp);
  return INT2FIX(tp_GetAnalyzerPriority(*tp));
}

/*
 * Get the next notification message from the TunePimp::TunePimp
 * object's message queue.
 *
 * Returns an array with the message type and file id, or nil if there
 * are no pending messages.  Here's a list of possible values for
 * message type:
 *   TunePimp::Callback::FileAdded  
 *   TunePimp::Callback::FileChanged  
 *   TunePimp::Callback::FileRemoved  
 *   TunePimp::Callback::WriteTagsComplete  
 *   TunePimp::Callback::CallbackLast 
 *
 * Aliases:
 *   TunePimp::TunePimp#notification
 *   TunePimp::TunePimp#get_notification
 *
 * Example:
 *   msg_type, file_id = tp.notification
 *
 */
static VALUE tp_tp_not(VALUE self) {
  tunepimp_t *tp;
  TPCallbackEnum type;
  int file_id;
  VALUE ret;

  ret = Qnil;
  Data_Get_Struct(self, tunepimp_t, tp);
  if (tp_GetNotification(*tp, &type, &file_id)) {
    ret = rb_ary_new();
    rb_ary_push(ret, INT2FIX(type));
    rb_ary_push(ret, INT2FIX(file_id));
  }

  return ret;
}

/*
 * Get the next status message in the TunePimp::TunePimp status queue.
 *
 * Returns nil if there is no pending status message.
 *
 * Aliases:
 *   TunePimp::TunePimp#status
 *   TunePimp::TunePimp#get_status
 * 
 * Example:
 *   puts 'Status: ' << tp.status
 *
 */
static VALUE tp_tp_status(VALUE self) {
  tunepimp_t *tp;
  char status[1024];
  VALUE ret;

  ret = Qnil;
  Data_Get_Struct(self, tunepimp_t, tp);
  if (tp_GetStatus(*tp, status, 1024))
    ret = rb_str_new2(status);
  
  return ret;
}

/*
 * Get the last error from this TunePimp::TunePimp object.
 *
 * Aliases:
 *   TunePimp::TunePimp#error
 *   TunePimp::TunePimp#get_error
 * 
 * Example:
 *   puts 'Error: ' << tp.error
 *
 */
static VALUE tp_tp_error(VALUE self) {
  tunepimp_t *tp;
  char err[1024];
  Data_Get_Struct(self, tunepimp_t, tp);
  tp_GetError(*tp, err, 1024);
  return rb_str_new2(err);
}

/*
 * Send debugging information for this TunePimp::TunePimp object to
 * stdout?
 *
 * Example:
 *   tp.debug = true # enable debugging
 *
 */
static VALUE tp_tp_set_debug(VALUE self, VALUE debug) {
  tunepimp_t *tp;
  Data_Get_Struct(self, tunepimp_t, tp);
  tp_SetDebug(*tp, !(debug == Qfalse || debug == Qnil));
  return debug;
}

/*
 * Is debugging enabled for this TunePimp::TunePimp object?
 *
 * Example:
 *   puts 'Debugging is ' << tp.debug ? 'enabled' : 'disabled'
 *
 */
static VALUE tp_tp_debug(VALUE self) {
  tunepimp_t *tp;
  Data_Get_Struct(self, tunepimp_t, tp);
  return tp_GetDebug(*tp) ? Qtrue : Qfalse;
}

/*
 * Add a file to this TunePimp::TunePimp object's file list.
 *
 * Returns the ID of the file added.  Note that this method always
 * succeeds, even if the file doesn't exist or is inaccessible (although
 * in those cases, there will be an error in TunePimp::TunePimp#error).
 *
 * Example:
 *   id = tp.add_file('test.mp3')
 *
 */
static VALUE tp_tp_add_file(VALUE self, VALUE path) {
  tunepimp_t *tp;
  Data_Get_Struct(self, tunepimp_t, tp);
  return INT2FIX(tp_AddFile(*tp, RSTRING(path)->ptr));
}

/*
 * Add a directory to this TunePimp::TunePimp object's file list.
 *
 * Returns the number of the files added.  If a file already exists in
 * the list, it is ignored.
 *
 * Example:
 *   num = tp.add_dir('My Music')
 *   puts "Added #{num} files."
 *
 */
static VALUE tp_tp_add_dir(VALUE self, VALUE path) {
  tunepimp_t *tp;
  Data_Get_Struct(self, tunepimp_t, tp);
  return INT2FIX(tp_AddDir(*tp, RSTRING(path)->ptr));
}

/*
 * Remove a file from this TunePimp::TunePimp object's file list.
 *
 * Example:
 *   id = tp.add_file('test.mp3')
 *   tp.remove(id)
 *
 */
static VALUE tp_tp_remove(VALUE self, VALUE file_id) {
  tunepimp_t *tp;
  Data_Get_Struct(self, tunepimp_t, tp);
  tp_Remove(*tp, NUM2INT(file_id));
  return Qnil;
}

/*
 * Get the number of files in this TunePimp::TunePimp object's file list.
 *
 * Aliases:
 *   TunePimp::TunePimp#num_files
 *   TunePimp::TunePimp#get_num_files
 *
 * Example:
 *   puts "There are #{tp.num_files} files."
 *
 */
static VALUE tp_tp_num_files(VALUE self) {
  tunepimp_t *tp;
  Data_Get_Struct(self, tunepimp_t, tp);
  return INT2FIX(tp_GetNumFiles(*tp));
}

/*
 * Get the number of TRMs in this TunePimp::TunePimp object's that
 * haven't been submitted to the server yet.
 *
 * Aliases:
 *   TunePimp::TunePimp#num_unsubmitted
 *   TunePimp::TunePimp#get_num_unsubmitted
 *
 * Example:
 *   puts "There are #{tp.num_unsubmitted} unsubmitted TRMs."
 *
 */
static VALUE tp_tp_num_unsub(VALUE self) {
  tunepimp_t *tp;
  Data_Get_Struct(self, tunepimp_t, tp);
  return INT2FIX(tp_GetNumUnsubmitted(*tp));
}

/*
 * Get the number of unsaved items in this TunePimp::TunePimp object.
 *
 * Aliases:
 *   TunePimp::TunePimp#num_unsaved_items
 *   TunePimp::TunePimp#get_num_unsaved_items
 *
 * Example:
 *   puts "There are #{tp.num_unsaved_items} unsaved items."
 *
 */
static VALUE tp_tp_num_unsaved_items(VALUE self) {
  tunepimp_t *tp;
  Data_Get_Struct(self, tunepimp_t, tp);
  return INT2FIX(tp_GetNumUnsavedItems(*tp));
}

/*
 * Get the number of tracks in each status category in this
 * TunePimp::TunePimp object.
 *
 * Returns an array that can be indexed via TunePimp::Status.  Here's a
 * brief description of each TunePimp::Status type:
 *
 *   * TunePimp::Status::Unrecognized: TunePimp was not able to match
 *     this track.
 *   * TunePimp::Status::Recognized: Tunepimp recognized this track.
 *   * TunePimp::Status::Pending: This track is waiting for TunePimp to
 *     process it.
 *   * TunePimp::Status::TRMLookup: TunePimp is performing a TRM lookup
 *     on this track.
 *   * TunePimp::Status::TRMCollision: TunePimp has detected a TRM
 *     collision for this track. The user must select which of the
 *     matching TRMs to use to finish the recognition process.
 *   * TunePimp::Status::FileLookup: TunePimp is doing a metadata lookup
 *     on this file.
 *   * TunePimp::Status::UserSelection: The MB server has returned more
 *     than one possible match for the track. The user must select a
 *     match to continue the lookup process.
 *   * TunePimp::Status::Verified: The user has verified the matched
 *     track and TunePimp is getting ready to save the changes to disk.
 *   * TunePimp::Status::Deleted: This track has been deleted from
 *     TunePimp, but its reference count has not reached zero yet.
 *   * TunePimp::Status::Error: An error occured during the lookup of
 *     this track. 
 *
 * Example:
 *   num = tp.track_counts(TunePimp::Status::Recognized)
 *   puts "There are #{num} recognized tracks."
 *
 */
static VALUE tp_tp_track_counts(VALUE self) {
  tunepimp_t *tp;
  int i, counts[eLastStatus];
  VALUE ret;

  ret = Qnil;
  Data_Get_Struct(self, tunepimp_t, tp);
  if (tp_GetTrackCounts(*tp, counts, eLastStatus)) {
    ret = rb_ary_new();
    for (i = 0; i < eLastStatus; i++)
      rb_ary_push(ret, INT2FIX(counts[i]));
  }

  return ret;
}

/*
 * Return the number of files in this TunePimp::TunePimp object's file
 * list.
 *
 * Aliases:
 *   TunePimp::TunePimp#num_file_ids
 *   TunePimp::TunePimp#get_num_file_ids
 *
 * Example:
 *   puts "There are #{tp.num_file_ids} files in this list."
 *
 */
static VALUE tp_tp_num_file_ids(VALUE self) {
  tunepimp_t *tp;
  Data_Get_Struct(self, tunepimp_t, tp);
  return INT2FIX(tp_GetNumFileIds(*tp));
}

/* 
 * Get the file ids in this TunePimp::TunePimp object's file list.
 *
 * Aliases:
 *   TunePimp::TunePimp#file_ids
 *   TunePimp::TunePimp#get_file_ids
 * 
 * Example:
 *   puts "File IDs: #{tp.file_ids.join(',')}"
 *
 */
static VALUE tp_tp_file_ids(VALUE self) {
  tunepimp_t *tp;
  int i, *ids, num, size;
  VALUE ret;

  Data_Get_Struct(self, tunepimp_t, tp);
  num = tp_GetNumFileIds(*tp);
  size = sizeof(int) * num;
  if ((ids = malloc(size)) == NULL)
    rb_raise(eException, "Couldn't alloc %d bytes for int*", size);
  tp_GetFileIds(*tp, ids, num);

  ret = rb_ary_new();
  for (i = 0; i < num; i++)
    rb_ary_push(ret, INT2FIX(ids[i]));
  free(ids);

  return ret;
}

/*
 * Get the TunePimp::Track associated with a given file ID.
 *
 * Returns nil if the given file ID is invalid.
 *
 * Example:
 *   track = tp.get_track(file_id)
 *
 */
static VALUE tp_tp_track(VALUE self, VALUE file_id) {
  tunepimp_t *tp;
  track_t *tr;
  VALUE track;

  if ((tr = malloc(sizeof(track_t))) == NULL)
    rb_raise(eException, "Couldn't allocate %d bytes for track_t", sizeof(track_t));

  Data_Get_Struct(self, tunepimp_t, tp);
  if ((*tr = tp_GetTrack(*tp, NUM2INT(file_id))) != NULL) {
    track = Data_Wrap_Struct(cTr, 0, tp_tp_free, tr);
    rb_obj_call_init(track, 0, NULL);
  } else {
    free(tr);
    track = Qnil;
  }
  
  return track;
}

/*
 * Release TunePimp::Track from TunePimp::TunePimp object.
 *
 * Example:
 *   tp.release_track(track)
 *
 */
static VALUE tp_tp_release_track(VALUE self, VALUE track) {
  tunepimp_t *tp;
  track_t *tr;

  Data_Get_Struct(self, tunepimp_t, tp);
  Data_Get_Struct(track, track_t, tr);
  tp_ReleaseTrack(*tp, *tr);

  return Qnil;
}

/*
 * Wake up this TunePimp::TunePimp object to look for work to do.
 *
 * Note: You have to call this every time you change a TunePimp::Track's
 * status with TunePimp::Track#set_status, or the changes won't be
 * saved.
 *
 * Example:
 *   tp.wake(track)
 *
 */
static VALUE tp_tp_wake(VALUE self, VALUE track) {
  tunepimp_t *tp;
  track_t *tr;

  Data_Get_Struct(self, tunepimp_t, tp);
  Data_Get_Struct(track, track_t, tr);
  tp_Wake(*tp, *tr);

  return Qnil;
}

/*
 * Select a result returned by TunePimp::Track#results.
 *
 * Returns TunePimp::Error:InvalidIndex if index is invalid
 *
 * Example:
 *   tp.select_result(track, 5)
 *
 */
static VALUE tp_tp_select_result(VALUE self, VALUE track, VALUE idx) {
  tunepimp_t *tp;
  track_t *tr;

  Data_Get_Struct(self, tunepimp_t, tp);
  Data_Get_Struct(track, track_t, tr);
  return INT2FIX(tp_SelectResult(*tp, *tr, NUM2INT(idx)));
}

/*
 * Discard all MusicBrainz idenitifiers and identify this file again.
 * The status is set back to TunePimp::Status::Pending which causes the
 * whole identification process to start again, including TRM
 * generation.
 *
 * Example:
 *   tp.misidentified(id)
 *
 */
static VALUE tp_tp_misidentified(VALUE self, VALUE file_id) {
  tunepimp_t *tp;
  Data_Get_Struct(self, tunepimp_t, tp);
  tp_Misidentified(*tp, NUM2INT(file_id));
  return Qnil;
}

/*
 * Mark a file as misidentified. The status is set back to
 * TunePimp::Status::FileLookup which causes file lookup to be done on
 * the file. Any MusicBrainz ids that have been downloaded for this file
 * are ignored and a lookup from the metadata is done in order to find
 * the right file on the MB server.
 *
 * Example:
 *   tp.identify_again(id)
 *
 */
static VALUE tp_tp_identify_again(VALUE self, VALUE file_id) {
  tunepimp_t *tp;
  Data_Get_Struct(self, tunepimp_t, tp);
  tp_IdentifyAgain(*tp, NUM2INT(file_id));
  return Qnil;
}

/*
 * Write tags for specified file IDs.  Only files in the state
 * TunePimp::Status::Recognized will be written.  If
 * TunePimp::TunePimp::rename_files is enabled, the files are also
 * renamed.
 *
 * Returns false if a file ID was invalid or if not all the files were
 * in the state TunePimp::Status::Recognized.
 *
 * Examples:
 *   # write tags for files with the specified file IDs
 *   tp.write_tags(1, 5 , 5)
 *
 *   # write tags for all files in Recognized state
 *   tp.write_tags
 *
 */
static VALUE tp_tp_write_tags(int argc, VALUE *argv, VALUE self) {
  tunepimp_t *tp;
  int i, *ids, size, r;

  ids = NULL;
  if (argc > 0) {
    size = sizeof(int) * argc;
    if ((ids = malloc(size)) == NULL)
      rb_raise(eException, "Couldn't alloc %d bytes for int*", size);
    for (i = 0; i < argc; i++)
      ids[i] = NUM2INT(argv[i]);
  }

  Data_Get_Struct(self, tunepimp_t, tp);
  r = INT2FIX(tp_WriteTags(*tp, ids, argc)) ? Qtrue : Qfalse;
  if (ids)
    free(ids);
  
  return r ? Qtrue : Qfalse;
}

/*
 * Add a track ID, TRM pair to the unsubmitted TRM queue.  You'll have
 * to call TunePimp::TunePimp#submit_trms to actually submit the queue.
 *
 * Aliases:
 *   TunePimp::TunePimp#add_trm
 *   TunePimp::TunePimp#add_trm_submission
 *
 * Example:
 *   tp.add_trm(track, trm)
 *
 */
static VALUE tp_tp_add_trm(VALUE self, VALUE tr_id, VALUE trm_id) {
  tunepimp_t *tp;
  Data_Get_Struct(self, tunepimp_t, tp);
  tp_AddTRMSubmission(*tp, RSTRING(tr_id)->ptr, RSTRING(trm_id)->ptr);
  return Qnil;
}

/*
 * Submit queued TRMs to musicbrainz.
 *
 * Returns TunePimp::Error::Ok on success and
 * TunePimp::Error::SubmitError on error.
 *
 * Example:
 *   tp.submit_trms
 *
 */
static VALUE tp_tp_submit_trms(VALUE self) {
  tunepimp_t *tp;
  Data_Get_Struct(self, tunepimp_t, tp);
  return INT2FIX(tp_SubmitTRMs(*tp));
}

/*
 * Rename files?  If enabled, files are renamed as specified in the
 * TunePimp::TunePimp#file_mask file mask.
 *
 * Enabled by default.
 *
 * Example:
 *   tp.rename_files = true
 *
 */
static VALUE tp_tp_set_rename_files(VALUE self, VALUE rename) {
  tunepimp_t *tp;
  Data_Get_Struct(self, tunepimp_t, tp);
  tp_SetRenameFiles(*tp, !(rename == Qfalse || rename == Qnil));
  return Qnil;
}

/*
 * Get the TunePimp::TunePimp#rename_files flag.
 *
 * Example:
 *   puts "Renaming is " + (tp.rename_files ? : 'on' : 'off')
 *
 */
static VALUE tp_tp_rename_files(VALUE self) {
  tunepimp_t *tp;
  Data_Get_Struct(self, tunepimp_t, tp);
  return tp_GetRenameFiles(*tp) ? Qtrue : Qfalse;
}

/*
 * Move files?  If enabled, files are moved as specified in the
 * TunePimp::TunePimp#file_mask file mask.
 *
 * Enabled by default.
 *
 * Example:
 *   tp.move_files = true
 *
 */
static VALUE tp_tp_set_move_files(VALUE self, VALUE move) {
  tunepimp_t *tp;
  Data_Get_Struct(self, tunepimp_t, tp);
  tp_SetMoveFiles(*tp, !(move == Qfalse || move == Qnil));
  return Qnil;
}

/*
 * Get the TunePimp::TunePimp#move_files flag.
 *
 * Example:
 *   puts "Moving is " + (tp.move_files ? : 'on' : 'off')
 *
 */
static VALUE tp_tp_move_files(VALUE self) {
  tunepimp_t *tp;
  Data_Get_Struct(self, tunepimp_t, tp);
  return tp_GetMoveFiles(*tp) ? Qtrue : Qfalse;
}

/*
 * Write ID3v1 tags?  
 *
 * Enabled by default.
 *
 * Example:
 *   tp.write_id3v1 = true
 *
 */
static VALUE tp_tp_set_write_id3v1(VALUE self, VALUE id3) {
  tunepimp_t *tp;
  Data_Get_Struct(self, tunepimp_t, tp);
  tp_SetWriteID3v1(*tp, !(id3 == Qfalse || id3 == Qnil));
  return Qnil;
}

/*
 * Get the TunePimp::TunePimp#write_id3v1 flag.
 *
 * Example:
 *   puts "ID3v1 is " + (tp.write_id3v1 ? : 'on' : 'off')
 *
 */
static VALUE tp_tp_write_id3v1(VALUE self) {
  tunepimp_t *tp;
  Data_Get_Struct(self, tunepimp_t, tp);
  return tp_GetWriteID3v1(*tp) ? Qtrue : Qfalse;
}

/*
 * Clear tags before MusicBrainz writes new ones?
 *
 * Disabled by default.
 *
 * Example:
 *   tp.clear_tags = true
 *
 */
static VALUE tp_tp_set_clear_tags(VALUE self, VALUE clear_tags) {
  tunepimp_t *tp;
  Data_Get_Struct(self, tunepimp_t, tp);
  tp_SetClearTags(*tp, !(clear_tags == Qfalse || clear_tags == Qnil));
  return Qnil;
}

/*
 * Get the TunePimp::TunePimp#clear_tags flag.
 *
 * Example:
 *   puts "Clear Tags is " + (tp.clear_tags ? : 'on' : 'off')
 *
 */
static VALUE tp_tp_clear_tags(VALUE self) {
  tunepimp_t *tp;
  Data_Get_Struct(self, tunepimp_t, tp);
  return tp_GetClearTags(*tp) ? Qtrue : Qfalse;
}

/*
 * Set the file mask used for renaming files.  
 *
 * Valid escape sequences:
 *   %artist (Name of the artist)
 *   %abc (The first character of the artist's sortname)
 *   %abc2 (The first two character of the artist's sortname)
 *   %abc3 (The first three character of the artist's sortname)
 *   %sortname (Sortname of the artist)
 *   %track (Title of the song)
 *   %album (Title of the album)
 *   %num (Track number on the album)
 *   %0num (Track number on the album, zero padded to two places)
 *   %format (The format of the given file e.g. ogg/mp3/wav/flac/ape)
 *   %type (The release type: single, album, remix, etc)
 *   %status (The release status: official, bootleg, promo)
 *   %year (The first release year)
 *   %month (The first release month)
 *   %day (The first release day)
 *   %country (The first release country)
 *
 * The default file mask is "%artist/%album/%artist-%album-%0num-%track".
 * 
 * Be careful about using % escape sequences in double-quoted Ruby
 * strings!
 *
 * Note: See the TunePimp::TunePimp#various_file_mask method as well.
 *
 * Example:
 *   # set the file mask (note the single-quoted string)
 *   tp.file_mask = '%sortname/%album/%0num. %track.%format'
 *   
 */
static VALUE tp_tp_set_file_mask(VALUE self, VALUE file_mask) {
  tunepimp_t *tp;
  Data_Get_Struct(self, tunepimp_t, tp);
  tp_SetFileMask(*tp, RSTRING(file_mask)->ptr);
  return Qnil;
}

/*
 * Get the TunePimp::TunePimp#file_mask.
 *
 * Example:
 *   puts "Mask: " << tp.file_mask
 *
 */
static VALUE tp_tp_file_mask(VALUE self) {
  tunepimp_t *tp;
  char buf[1024];
  Data_Get_Struct(self, tunepimp_t, tp);
  tp_GetFileMask(*tp, buf, 1024);
  return rb_str_new2(buf);
}

/*
 * Set the file mask used for renaming files in multi-artist albums.  
 *
 * Valid escape sequences:
 *   %artist (Name of the artist)
 *   %abc (The first character of the artist's sortname)
 *   %abc2 (The first two character of the artist's sortname)
 *   %abc3 (The first three character of the artist's sortname)
 *   %sortname (Sortname of the artist)
 *   %track (Title of the song)
 *   %album (Title of the album)
 *   %num (Track number on the album)
 *   %0num (Track number on the album, zero padded to two places)
 *   %format (The format of the given file e.g. ogg/mp3/wav/flac/ape)
 *   %type (The release type: single, album, remix, etc)
 *   %status (The release status: official, bootleg, promo)
 *   %year (The first release year)
 *   %month (The first release month)
 *   %day (The first release day)
 *   %country (The first release country)
 *
 * The default value for TunePimp::TunePimp#various_file_mask is 
 * "Various Artists/%album/%album-%0num-%artist-%track".
 * 
 * Be careful about using % escape sequences in double-quoted Ruby
 * strings!
 *
 * Note: See the TunePimp::TunePimp::file_mask method as well.
 *
 * Example:
 *   # set the file mask (note the single-quoted string)
 *   tp.file_mask = '%sortname/%album/%0num. %track.%format'
 *   
 */
static VALUE tp_tp_set_various_file_mask(VALUE self, VALUE various_file_mask) {
  tunepimp_t *tp;
  Data_Get_Struct(self, tunepimp_t, tp);
  tp_SetVariousFileMask(*tp, RSTRING(various_file_mask)->ptr);
  return Qnil;
}

/*
 * Get the TunePimp::TunePimp::various_file_mask.
 *
 * Example:
 *   puts "Mask: " << tp.various_file_mask
 *
 */
static VALUE tp_tp_various_file_mask(VALUE self) {
  tunepimp_t *tp;
  char buf[1024];
  Data_Get_Struct(self, tunepimp_t, tp);
  tp_GetVariousFileMask(*tp, buf, 1024);
  return rb_str_new2(buf);
}

/*
 * Set the characters allowed in file names.
 *
 * Note: '/' is always permitted.
 *
 * Defaults to "" (all characters allowed).
 *
 * Example:
 *   tp.set_allowed_file_chars 'abcdefghijklmnopqrstuvwxyz0123456789.-_ '
 *
 */
static VALUE tp_tp_set_allowed_file_chars(VALUE self, VALUE allowed_file_chars) {
  tunepimp_t *tp;
  Data_Get_Struct(self, tunepimp_t, tp);
  tp_SetAllowedFileCharacters(*tp, RSTRING(allowed_file_chars)->ptr);
  return Qnil;
}

/*
 * Get the TunePimp::TunePimp::allowed_file_chars value.
 * 
 * Example:
 *   puts "Allowed Characters: \"" << tp.allowed_file_chars << "\"."
 *
 */
static VALUE tp_tp_allowed_file_chars(VALUE self) {
  tunepimp_t *tp;
  char buf[1024];
  Data_Get_Struct(self, tunepimp_t, tp);
  tp_GetAllowedFileCharacters(*tp, buf, 1024);
  return rb_str_new2(buf);
}

/*
 * Set the base directory that is prepended to all file names if the
 * TunePimp::TunePimp#move_files flag is enabled.
 *
 * Defaults to "MyMusic" (in the current working directory).
 *
 * Example:
 *   tp.dest_dir = '/store/mp3'
 *
 */
static VALUE tp_tp_set_dest_dir(VALUE self, VALUE dest_dir) {
  tunepimp_t *tp;
  Data_Get_Struct(self, tunepimp_t, tp);
  tp_SetDestDir(*tp, RSTRING(dest_dir)->ptr);
  return Qnil;
}

/*
 * Get the value of TunePimp::TunePimp#dest_dir.
 *
 * Example:
 *   puts "Destination: " << tp.dest_dir
 *
 */
static VALUE tp_tp_dest_dir(VALUE self) {
  tunepimp_t *tp;
  char buf[1024];
  Data_Get_Struct(self, tunepimp_t, tp);
  tp_GetDestDir(*tp, buf, 1024);
  return rb_str_new2(buf);
}

/*
 * Set the top source directory for TunePimp::TunePimp object.
 * 
 * When TunePimp moves a file out of a directory to a new directory, and
 * the old directory is now empty, the old directory is removed. This
 * process is repeated for the old directory's parent, until it reaches
 * the top_src_dir, where it stops removing directories. Set the
 * top_src_dir to root of the directory tree where TunePimp is writing
 * ID3 files to.
 *
 * For example, if top_src_dir is set to "/mnt/mp3"
 *
 *   /mnt/mp3/dirty_mp3s/Beatles/Yesterday.mp3
 *
 * The Yesterday song is the only file left in the dirty_mp3s folder.
 * When tunepimp moves this file to its new, clean location, it will
 * remove the Beatles directory, then the dirty_mp3s directory, but it
 * will not remove the "/mnt/mp3" directory, since that is the
 * top_src_dir.
 *
 * Example:
 *   tp.top_src_dir = '/store/mp3'
 *
 */
static VALUE tp_tp_set_top_src_dir(VALUE self, VALUE top_src_dir) {
  tunepimp_t *tp;
  Data_Get_Struct(self, tunepimp_t, tp);
  tp_SetTopSrcDir(*tp, RSTRING(top_src_dir)->ptr);
  return Qnil;
}

/*
 * Get the top source directory for a TunePimp::TunePimp object.
 *
 * Example:
 *   puts "Dir: " << tp.top_src_dir
 *
 */
static VALUE tp_tp_top_src_dir(VALUE self) {
  tunepimp_t *tp;
  char buf[1024];
  Data_Get_Struct(self, tunepimp_t, tp);
  tp_GetTopSrcDir(*tp, buf, 1024);
  return rb_str_new2(buf);
}

/*
 * Set the TRM collision threshold for a TunePimp::TunePimp object.
 *
 * If there is more than one entry matching a TRM in the musicbrainz
 * database, we have a TRM collision. In this case, a similarity value
 * is calculated for each returned database entry using the file's meta
 * data as the reference. The similarity is a percent value describing
 * how good the match is.
 *
 * If the similarity is greater than or equal to the
 * trm_collision_threshold, then the file is marked as recognized
 * (TunePimp::Status::Recognized). Otherwise, the track's status is set
 * to TunePimp::Status::TRMCollision, which requires user invention in 
 * order to select one of the returned results.
 *
 * Valid values for the threshold are between 0 and 100, inclusive.
 *
 * Default: 80.
 *
 * Example:
 *   tp.trm_collision_threshold = 95
 *
 */
static VALUE tp_tp_set_trm_collision_threshold(VALUE self, VALUE trm_collision_threshold) {
  tunepimp_t *tp;
  Data_Get_Struct(self, tunepimp_t, tp);
  tp_SetTRMCollisionThreshold(*tp, NUM2INT(trm_collision_threshold));
  return Qnil;
}

/*
 * Get the TRM collision threshold for a TunePimp::TunePimp object.
 *
 * Example:
 *   puts "Threshold: #{tp.trm_collision_threshold}"
 *
 */
static VALUE tp_tp_trm_collision_threshold(VALUE self) {
  tunepimp_t *tp;
  Data_Get_Struct(self, tunepimp_t, tp);
  return INT2FIX(tp_GetTRMCollisionThreshold(*tp));
}

/*
 * Set the minimum recognition threshold for TRM lookups with the given
 * TunePimp::TunePimp object.
 *
 * If there is a TRM match for a given track, but the similarity between
 * the metadata returned is below the minimum recognition threshold,
 * then the TRM match should not be accepted and the file should be
 * listed as unrecognized.
 *
 * Valid values for the threshold are between 0 and 100, inclusive.
 *
 * Default: 50.
 *
 * Example:
 *   tp.min_trm_threshold = 10
 *
 */
static VALUE tp_tp_set_min_trm_threshold(VALUE self, VALUE min_trm_threshold) {
  tunepimp_t *tp;
  Data_Get_Struct(self, tunepimp_t, tp);
  tp_SetMinTRMThreshold(*tp, NUM2INT(min_trm_threshold));
  return Qnil;
}

/*
 * Get the minimum TRM threshold for a TunePimp::TunePimp object.
 *
 * Example:
 *   puts "min thresh: #{tp.min_trm_threshold}"
 *
 */
static VALUE tp_tp_min_trm_threshold(VALUE self) {
  tunepimp_t *tp;
  Data_Get_Struct(self, tunepimp_t, tp);
  return INT2FIX(tp_GetMinTRMThreshold(*tp));
}

/*
 * Set the auto-save threshold for a TunePimp::TunePimp object.
 *
 * When the similarity value (see TunePimp::TunePimp#trm_collision) is
 * greater or equal the auto save threshold, a recognized file (in state
 * TunePimp::Status::Recognized) is marked as verified
 * (TunePimp::Status::Verified). The effect is the same as calling
 * TunePimp::TunePimp#write_tags on a recognized file.
 *
 * The writing is done immediately, without the possibility of user
 * intervention.
 *
 * Valid values for the threshold are between 0 and 100, inclusive.
 * Set it to a negative value or nil in order to disable the auto-save
 * feature.
 *
 * Default: 90.
 *
 * Example:
 *   # disable auto-save
 *   tp.auto_save_threshold = nil
 *
 */
static VALUE tp_tp_set_auto_save_threshold(VALUE self, VALUE auto_save_threshold) {
  tunepimp_t *tp;
  int thresh;
  thresh = (auto_save_threshold == Qnil) ? -1 : NUM2INT(auto_save_threshold);
  Data_Get_Struct(self, tunepimp_t, tp);
  tp_SetAutoSaveThreshold(*tp, thresh);
  return Qnil;
}

/*
 * Get the auto-save threshold of a TunePimp::TunePimp object.
 *
 * Example:
 *   puts "auto-save threshold: #{tp.auto_save_threshold}"
 *
 */
static VALUE tp_tp_auto_save_threshold(VALUE self) {
  tunepimp_t *tp;
  Data_Get_Struct(self, tunepimp_t, tp);
  return INT2FIX(tp_GetAutoSaveThreshold(*tp));
}

/*
 * Set the maximum length of a path saved by this TunePimp::TunePimp
 * object.
 *
 * Example:
 *   tp.max_file_name_len = 128
 *
 */
static VALUE tp_tp_set_max_file_name_len(VALUE self, VALUE max_file_name_len) {
  tunepimp_t *tp;
  Data_Get_Struct(self, tunepimp_t, tp);
  tp_SetMaxFileNameLen(*tp, NUM2INT(max_file_name_len));
  return Qnil;
}

/*
 * Get the maximum length of a file name saved by this
 * TunePimp::TunePimp object.
 *
 * Example:
 *   puts "max length: #{tp.max_file_name_len}"
 *
 */
static VALUE tp_tp_max_file_name_len(VALUE self) {
  tunepimp_t *tp;
  Data_Get_Struct(self, tunepimp_t, tp);
  return INT2FIX(tp_GetMaxFileNameLen(*tp));
}

/*
 * Delete files after they've been saved?
 *
 * If false, the status of the track is set to TunePimp::Status::Saved
 * instead.
 *
 * Example:
 *   # let the user remove stuff
 *   tp.set_auto_remove_saved_files = false
 *
 */
static VALUE tp_tp_set_auto_remove_saved_files(VALUE self, VALUE auto_remove_saved_files) {
  tunepimp_t *tp;
  Data_Get_Struct(self, tunepimp_t, tp);
  tp_SetAutoRemovedSavedFiles(*tp, !(auto_remove_saved_files == Qfalse || auto_remove_saved_files == Qnil));
  return Qnil;
}

/*
 * Get the value of the TunePimp::TunePimp#auto_remove_saved_files
 * flag.
 *
 * Example:
 *   puts "Magic delete is " << (tp.auto_remove_saved_files ? 'on' : 'off')
 *
 */
static VALUE tp_tp_auto_remove_saved_files(VALUE self) {
  tunepimp_t *tp;
  Data_Get_Struct(self, tunepimp_t, tp);
  return tp_GetAutoRemovedSavedFiles(*tp) ? Qtrue : Qfalse;
}

/*
 * Get a list of files with status TunePimp::Status::Recognized, with a
 * similarity less than threshold. 
 *
 * Example:
 *   ary = tp.recognized_files(90)
 *
 */
static VALUE tp_tp_recognized_files(VALUE self, VALUE thresh) {
  tunepimp_t *tp;
  int i, *ids, num;
  VALUE ret;

  ret = Qnil;
  Data_Get_Struct(self, tunepimp_t, tp);
  if (tp_GetRecognizedFileList(*tp, NUM2INT(thresh), &ids, &num)) {
    ret = rb_ary_new();
    for (i = 0; i < num; i++)
      rb_ary_push(ret, INT2FIX(ids[i]));
    tp_DeleteRecognizedFileList(*tp, ids);
  }

  return ret;
}


/*********************************************************************/
/* TunePimp::Track methods                                           */
/*********************************************************************/

/*
 * You cannot instantiate this class directly.  Use
 * TunePimp::TunePimp#get_track instead.
 *
 */
VALUE tp_tr_new(VALUE klass) {
  rb_raise(eException, "You cannot instantiate this class directly.  Use TunePimp::TunePimp#get_track instead.");
  return Qnil;
}

/*
 * Constructor for TunePimp::Track object.
 *
 * This method is currently empty.  You should never call this method
 * directly unless you're instantiating a derived class (ie, you know
 * what you're doing).
 *
 */
static VALUE tp_tr_init(VALUE self) {
  return self;
}

/*
 * Get the TunePimp::Status of a TunePimp::Track object.
 *
 * Example:
 *   puts "Recognized" if tr.status == TunePimp::Status::Recognized
 *
 */
static VALUE tp_tr_status(VALUE self) {
  track_t *tr;
  Data_Get_Struct(self, track_t, tr);
  return INT2FIX(tr_GetStatus(*tr));
}

/*
 * Set the status of a TunePimp::Track object.
 *
 * Example:
 *   tr.status = TunePimp::Status::Verified
 *   
 */
static VALUE tp_tr_set_status(VALUE self, VALUE status) {
  track_t *tr;
  Data_Get_Struct(self, track_t, tr);
  tr_SetStatus(*tr, NUM2INT(status));
  return Qnil;
}

/*
 * Get the file name of a TunePimp::Track object.
 *
 * Example:
 *   puts "Filename: " << tr.filename
 *
 */
static VALUE tp_tr_filename(VALUE self) {
  track_t *tr;
  char buf[1024];
  Data_Get_Struct(self, track_t, tr);
  tr_GetFileName(*tr, buf, 1024);
  return rb_str_new2(buf);
}

/*
 * Get the TRM of a TunePimp::Track object.
 *
 * Example:
 *   puts "TRM: " << tr.trm
 *
 */
static VALUE tp_tr_trm(VALUE self) {
  track_t *tr;
  char buf[1024];
  Data_Get_Struct(self, track_t, tr);
  tr_GetTRM(*tr, buf, 1024);
  return rb_str_new2(buf);
}

/*
 * Get the local metadata of a TunePimp::Track object.
 *
 * TODO: better documentation here.
 *
 * Example:
 *   md = tr.local_metadata
 *
 */
static VALUE tp_tr_local_metadata(VALUE self) {
  track_t *tr;
  metadata_t **md;
  
  Data_Get_Struct(self, track_t, tr);
  if ((md = malloc(sizeof(metadata_t *))) == NULL)
    rb_raise(eException, "Couldn't alloc metadata_t*");
  *md = md_New();
  tr_GetLocalMetadata(*tr, *md);
  
  return Data_Wrap_Struct(cMD, 0, tp_md_free, md);
}

/*
 * Set the local metadata of a TunePimp::Track object.
 *
 * TODO: better documentation here.
 *
 * Example:
 *   tr.local_metadata = md
 *
 */
static VALUE tp_tr_set_local_metadata(VALUE self, VALUE metadata) {
  track_t *tr;
  metadata_t **md;
  
  Data_Get_Struct(self, track_t, tr);
  Data_Get_Struct(metadata, metadata_t *, md);
  tr_SetLocalMetadata(*tr, *md);

  return Qnil;
}

/*
 * Get the server metadata of a TunePimp::Track object.
 *
 * TODO: better documentation here.
 *
 * Example:
 *   md = tr.server_metadata
 *
 */
static VALUE tp_tr_server_metadata(VALUE self) {
  track_t *tr;
  metadata_t **md;
  
  Data_Get_Struct(self, track_t, tr);
  if ((md = malloc(sizeof(metadata_t *))) == NULL)
    rb_raise(eException, "Couldn't alloc metadata_t*");
  *md = md_New();
  tr_GetServerMetadata(*tr, *md);
  
  return Data_Wrap_Struct(cMD, 0, tp_md_free, md);
}

/*
 * Set the server metadata of a TunePimp::Track object.
 *
 * TODO: better documentation here.
 *
 * Example:
 *   tr.server_metadata = md
 *
 */
static VALUE tp_tr_set_server_metadata(VALUE self, VALUE metadata) {
  track_t *tr;
  metadata_t **md;
  
  Data_Get_Struct(self, track_t, tr);
  Data_Get_Struct(metadata, metadata_t *, md);
  tr_SetServerMetadata(*tr, *md);

  return Qnil;
}

/*
 * Get the error of a TunePimp::Track object.
 *
 * Example:
 *   puts "Error: " << tr.error
 *
 */
static VALUE tp_tr_error(VALUE self) {
  track_t *tr;
  char buf[1024];
  Data_Get_Struct(self, track_t, tr);
  tr_GetError(*tr, buf, 1024);
  return rb_str_new2(buf);
}

/*
 * Get the similarity of a TunePimp::Track object.
 *
 * Example:
 *   puts "Similarity: #{tr.similarity}"
 *
 */
static VALUE tp_tr_similarity(VALUE self) {
  track_t *tr;
  Data_Get_Struct(self, track_t, tr);
  return INT2FIX(tr_GetSimilarity(*tr));
}

/*
 * Has this TunePimp::Track object changed?
 *
 * Example:
 *   puts 'This track has changed.' if tr.has_changed?
 *
 */
static VALUE tp_tr_has_changed(VALUE self) {
  track_t *tr;
  Data_Get_Struct(self, track_t, tr);
  return tr_HasChanged(*tr) ? Qtrue : Qfalse;
}

/*
 * Get the number of results for this track.
 *
 * Example:
 *   puts "Found #{tr.num_results} results."
 *
 */
static VALUE tp_tr_num_results(VALUE self) {
  track_t *tr;
  Data_Get_Struct(self, track_t, tr);
  return INT2FIX(tr_GetNumResults(*tr));
}

static VALUE wrap_artist(result_t *result) {
  artistresult_t *a_r;
  VALUE ret;

  a_r = (artistresult_t*) result;
  ret = rb_hash_new();
  rb_hash_aset(ret, rb_str_new2("relevance"), INT2FIX(a_r->relevance));
  rb_hash_aset(ret, rb_str_new2("id"), rb_str_new2(a_r->id));
  rb_hash_aset(ret, rb_str_new2("name"), rb_str_new2(a_r->name));
  rb_hash_aset(ret, rb_str_new2("sort_name"), rb_str_new2(a_r->sortName));

  return ret;
}

static VALUE wrap_album(result_t *result) {
  albumresult_t *a_r;
  VALUE ret;

  a_r = (albumresult_t*) result;
  ret = rb_hash_new();
  rb_hash_aset(ret, rb_str_new2("relevance"), INT2FIX(a_r->relevance));
  rb_hash_aset(ret, rb_str_new2("name"), rb_str_new2(a_r->name));
  rb_hash_aset(ret, rb_str_new2("id"), rb_str_new2(a_r->id));
  rb_hash_aset(ret, rb_str_new2("num_tracks"), INT2FIX(a_r->numTracks));
  rb_hash_aset(ret, rb_str_new2("num_cd_index_ids"), INT2FIX(a_r->numCDIndexIds));
  rb_hash_aset(ret, rb_str_new2("is_va"), a_r->isVA ? Qtrue : Qfalse);
  rb_hash_aset(ret, rb_str_new2("type"), INT2FIX(a_r->type));
  rb_hash_aset(ret, rb_str_new2("release_year"), INT2FIX(a_r->releaseYear));
  rb_hash_aset(ret, rb_str_new2("release_month"), INT2FIX(a_r->releaseMonth));
  rb_hash_aset(ret, rb_str_new2("release_day"), INT2FIX(a_r->releaseDay));
  rb_hash_aset(ret, rb_str_new2("release_country"), rb_str_new2(a_r->releaseCountry));
  rb_hash_aset(ret, rb_str_new2("artist"), wrap_artist((result_t*) a_r->artist));

  return ret;
}

static VALUE wrap_track(result_t *result) {
  albumtrackresult_t *a_r;
  VALUE ret;

  a_r = (albumtrackresult_t*) result;
  ret = rb_hash_new();
  rb_hash_aset(ret, rb_str_new2("relevance"), INT2FIX(a_r->relevance));
  rb_hash_aset(ret, rb_str_new2("name"), rb_str_new2(a_r->name));
  rb_hash_aset(ret, rb_str_new2("id"), rb_str_new2(a_r->id));
  rb_hash_aset(ret, rb_str_new2("num_trm_ids"), INT2FIX(a_r->numTRMIds));
  rb_hash_aset(ret, rb_str_new2("track_num"), INT2FIX(a_r->trackNum));
  rb_hash_aset(ret, rb_str_new2("duration"), INT2FIX(a_r->duration));
  rb_hash_aset(ret, rb_str_new2("artist"), wrap_artist((result_t*) a_r->artist));
  rb_hash_aset(ret, rb_str_new2("album"), wrap_album((result_t*) a_r->album));

  return ret;
}

/*
 * Get the results for this TunePimp::Track object.
 *
 * TODO: documentation
 *
 * Example: 
 *   type, results = tr.results
 *
 */
static VALUE tp_tr_results(VALUE self) {
  track_t *tr;
  TPResultType type;
  result_t results;
  int i, num;
  VALUE ary, ret;
  Data_Get_Struct(self, track_t, tr);

  ary = rb_ary_new();
  tr_GetResults(*tr, &type, &results, &num);
  switch (type) {
    case eArtistList:
      for (i = 0; i < num; i++)
        rb_ary_push(ary, wrap_artist(results + i));
      break;
    case eAlbumList:
      for (i = 0; i < num; i++)
        rb_ary_push(ary, wrap_album(results + i));
      break;
    case eTrackList:
      for (i = 0; i < num; i++)
        rb_ary_push(ary, wrap_track(results + i));
      break;
    case eNone:
    case eMatchedTrack:
/* 
*         ary = wrap_tracks(results, num);
*         break;
*/ 
    default:
      rb_raise(eException, "Result type %d not implemented", type);
  }
  rs_Delete(type, &results, num);

  ret = rb_ary_new();
  rb_ary_push(ret, INT2FIX(type));
  rb_ary_push(ret, ary);

  return ret;
}

/*
 * Lock this TunePimp::Track object.
 *
 * TODO: better documentation
 *
 * Example:
 *   tr.lock
 *
 */
static VALUE tp_tr_lock(VALUE self) {
  track_t *tr;
  Data_Get_Struct(self, track_t, tr);
  tr_Lock(*tr);
  return Qnil;
}

/*
 * Unlock this TunePimp::Track object.
 *
 * TODO: better documentation
 *
 * Example:
 *   tr.unlock
 *
 */
static VALUE tp_tr_unlock(VALUE self) {
  track_t *tr;
  Data_Get_Struct(self, track_t, tr);
  tr_Unlock(*tr);
  return Qnil;
}


/*********************************************************************/
/* TunePimp::Metadata methods                                        */
/*********************************************************************/
/*
 * Create a new TunePimp::Metadata object.
 *
 * Example:
 *   md = TunePimp::Metadata.new
 *
 */
VALUE tp_md_new(VALUE klass) {
  metadata_t **md;
  VALUE self;

  if ((md = malloc(sizeof(metadata_t*))) == NULL)
    rb_raise(eException, "Couldn't allocate memory for metadata_t*");
  *md = md_New();

  self = Data_Wrap_Struct(klass, 0, tp_md_free, md);
  rb_obj_call_init(self, 0, NULL);

  return self;
}

/*
 * Constructor for TunePimp::Metadata object.
 *
 * This method is currently empty.  You should never call this method
 * directly unless you're instantiating a derived class (ie, you know
 * what you're doing).
 *
 */
static VALUE tp_md_init(VALUE self) {
  return self;
}


/*********************************************************************/
/* End Shenanigans, begin init code.                                 */
/*********************************************************************/
void Init_tunepimp(void) {
  /**************************/
  /* define TunePimp module */
  /**************************/
  mTP = rb_define_module("TunePimp");
  rb_define_const(mTP, "VERSION", rb_str_new2(VERSION));
  
  /************************************/
  /* define TunePimp::Exception class */
  /************************************/
  eException = rb_define_class_under(mTP, "Exception", rb_eStandardError);

  /***********************************/
  /* define TunePimp::TunePimp class */
  /***********************************/
  cTP = rb_define_class_under(mTP, "TunePimp", rb_cObject);
  rb_define_singleton_method(cTP, "new", tp_tp_new, -1);
  rb_define_singleton_method(cTP, "initialize", tp_tp_init, 0);

  rb_define_method(cTP, "version", tp_tp_ver, 0);

  rb_define_method(cTP, "set_user_info", tp_tp_set_user_info, 2);
  rb_define_method(cTP, "user_info", tp_tp_get_user_info, 0);
  rb_define_alias(cTP, "get_user_info", "user_info");

  rb_define_method(cTP, "use_utf8=", tp_tp_set_use_utf8, 1);
  rb_define_method(cTP, "use_utf8", tp_tp_get_use_utf8, 0);

  rb_define_method(cTP, "set_server", tp_tp_set_server, 2);
  rb_define_method(cTP, "server", tp_tp_get_server, 0);
  rb_define_alias(cTP, "get_server", "server");

  rb_define_method(cTP, "set_proxy", tp_tp_set_proxy, 2);
  rb_define_method(cTP, "proxy", tp_tp_get_proxy, 0);
  rb_define_alias(cTP, "get_proxy", "proxy");

  rb_define_method(cTP, "num_supported_extensions", tp_tp_num_exts, 0);
  rb_define_alias(cTP, "get_num_supported_extensions", "num_supported_extensions");
  rb_define_method(cTP, "supported_extensions", tp_tp_exts, 0);
  rb_define_alias(cTP, "get_supported_extensions", "supported_extensions");

  rb_define_method(cTP, "analyzer_priority=", tp_tp_set_analyzer_prio, 1);
  rb_define_method(cTP, "analyzer_priority", tp_tp_analyzer_prio, 0);

  rb_define_method(cTP, "notification", tp_tp_not, 0); 
  rb_define_alias(cTP, "get_notification", "notification");
  
  rb_define_method(cTP, "status", tp_tp_status, 0); 
  rb_define_alias(cTP, "get_status", "status");
  
  rb_define_method(cTP, "error", tp_tp_error, 0); 
  rb_define_alias(cTP, "get_error", "error");
  
  rb_define_method(cTP, "debug=", tp_tp_set_debug, 1);
  rb_define_method(cTP, "debug", tp_tp_debug, 0);

  rb_define_method(cTP, "add_file", tp_tp_add_file, 1);
  rb_define_method(cTP, "add_dir", tp_tp_add_dir, 1);
  rb_define_method(cTP, "remove", tp_tp_remove, 1);

  rb_define_method(cTP, "num_files", tp_tp_num_files, 0);
  rb_define_alias(cTP, "get_num_files", "num_files");

  rb_define_method(cTP, "num_unsubmitted", tp_tp_num_unsub, 0);
  rb_define_alias(cTP, "get_num_unsubmitted", "num_unsubmitted");

  rb_define_method(cTP, "num_unsaved_items", tp_tp_num_unsaved_items, 0);
  rb_define_alias(cTP, "get_num_unsaved_items", "num_unsaved_items");

  rb_define_method(cTP, "track_counts", tp_tp_track_counts, 0);

  rb_define_method(cTP, "num_file_ids", tp_tp_num_file_ids, 0);
  rb_define_alias(cTP, "get_num_file_ids", "num_file_ids");
  rb_define_method(cTP, "file_ids", tp_tp_file_ids, 0);
  
  rb_define_method(cTP, "track", tp_tp_track, 1);
  rb_define_alias(cTP, "get_track", "track");
  rb_define_method(cTP, "release_track", tp_tp_release_track, 1);
  rb_define_method(cTP, "wake", tp_tp_wake, 1);
  rb_define_method(cTP, "select_result", tp_tp_select_result, 2);

  rb_define_method(cTP, "misidentified", tp_tp_misidentified, 1);
  rb_define_method(cTP, "identify_again", tp_tp_identify_again, 1);
  rb_define_method(cTP, "write_tags", tp_tp_write_tags, -1);
  rb_define_method(cTP, "add_trm", tp_tp_add_trm, 2);
  rb_define_alias(cTP, "add_trm_submission", "add_trm");
  rb_define_method(cTP, "submit_trms", tp_tp_submit_trms, 0);
  
  rb_define_method(cTP, "rename_files=", tp_tp_set_rename_files, 1);
  rb_define_method(cTP, "rename_files", tp_tp_rename_files, 0);
  
  rb_define_method(cTP, "move_files=", tp_tp_set_move_files, 1);
  rb_define_method(cTP, "move_files", tp_tp_move_files, 0);
  
  rb_define_method(cTP, "write_id3v1=", tp_tp_set_write_id3v1, 1);
  rb_define_method(cTP, "write_id3v1", tp_tp_write_id3v1, 0);
  
  rb_define_method(cTP, "clear_tags=", tp_tp_set_clear_tags, 1);
  rb_define_method(cTP, "clear_tags", tp_tp_clear_tags, 0);
  
  rb_define_method(cTP, "file_mask=", tp_tp_set_file_mask, 1);
  rb_define_method(cTP, "file_mask", tp_tp_file_mask, 0);
  
  rb_define_method(cTP, "various_file_mask=", tp_tp_set_various_file_mask, 1);
  rb_define_method(cTP, "various_file_mask", tp_tp_various_file_mask, 0);
  
  rb_define_method(cTP, "allowed_file_chars=", tp_tp_set_allowed_file_chars, 1);
  rb_define_method(cTP, "allowed_file_chars", tp_tp_allowed_file_chars, 0);
  
  rb_define_method(cTP, "dest_dir=", tp_tp_set_dest_dir, 1);
  rb_define_method(cTP, "dest_dir", tp_tp_dest_dir, 0);
  
  rb_define_method(cTP, "top_src_dir=", tp_tp_set_top_src_dir, 1);
  rb_define_method(cTP, "top_src_dir", tp_tp_top_src_dir, 0);
  
  rb_define_method(cTP, "trm_collision_threshold=", tp_tp_set_trm_collision_threshold, 1);
  rb_define_method(cTP, "trm_collision_threshold", tp_tp_trm_collision_threshold, 0);
  
  rb_define_method(cTP, "min_trm_threshold=", tp_tp_set_min_trm_threshold, 1);
  rb_define_method(cTP, "min_trm_threshold", tp_tp_min_trm_threshold, 0);
  
  rb_define_method(cTP, "auto_save_threshold=", tp_tp_set_auto_save_threshold, 1);
  rb_define_method(cTP, "auto_save_threshold", tp_tp_auto_save_threshold, 0);
  
  rb_define_method(cTP, "max_file_name_len=", tp_tp_set_max_file_name_len, 1);
  rb_define_method(cTP, "max_file_name_len", tp_tp_max_file_name_len, 0);
  
  rb_define_method(cTP, "auto_remove_saved_files=", tp_tp_set_auto_remove_saved_files, 1);
  rb_define_method(cTP, "auto_remove_saved_files", tp_tp_auto_remove_saved_files, 0);
  
  rb_define_method(cTP, "recognized_files", tp_tp_recognized_files, 1);
  
  /********************************/
  /* define TunePimp::Track class */
  /********************************/
  cTr = rb_define_class_under(mTP, "Track", rb_cObject);
  rb_define_singleton_method(cTr, "new", tp_tr_new, 0);
  rb_define_singleton_method(cTr, "initialize", tp_tr_init, 0);

  rb_define_method(cTr, "status", tp_tr_status, 0);
  rb_define_method(cTr, "status=", tp_tr_set_status, 1);

  rb_define_method(cTr, "filename", tp_tr_filename, 0);
  rb_define_method(cTr, "trm", tp_tr_trm, 0);

  rb_define_method(cTr, "local_metadata", tp_tr_local_metadata, 0);
  rb_define_method(cTr, "local_metadata=", tp_tr_set_local_metadata, 0);

  rb_define_method(cTr, "server_metadata", tp_tr_server_metadata, 0);
  rb_define_method(cTr, "server_metadata=", tp_tr_set_server_metadata, 0);
  
  rb_define_method(cTr, "error", tp_tr_error, 0);
  rb_define_method(cTr, "similarity", tp_tr_similarity, 0);
  rb_define_method(cTr, "has_changed?", tp_tr_has_changed, 0);
  rb_define_method(cTr, "num_results", tp_tr_num_results, 0);
  rb_define_method(cTr, "results", tp_tr_results, 0);
  rb_define_method(cTr, "lock", tp_tr_lock, 0);
  rb_define_method(cTr, "unlock", tp_tr_unlock, 0);

  /***********************************/
  /* define TunePimp::Metadata class */
  /***********************************/
  cMD = rb_define_class_under(mTP, "Metadata", rb_cObject);
  rb_define_singleton_method(cMD, "new", tp_md_new, 0);
  rb_define_singleton_method(cMD, "initialize", tp_md_init, 0);

/* 
 *   rb_define_singleton_method(cMD, "convert_to_album_status", tp_md_convert_to_album_status, 1);
 *   rb_define_singleton_method(cMD, "convert_to_album_type", tp_md_convert_to_album_type, 1);
 * 
 *   rb_define_singleton_method(cMD, "convert_from_album_status", tp_md_convert_from_album_status, 1);
 *   rb_define_singleton_method(cMD, "convert_from_album_type", tp_md_convert_from_album_type, 1);
 */ 
  
  /******************************************/
  /* define TunePimp::ThreadPriority module */
  /******************************************/
  mPrio = rb_define_module_under(mTP, "ThreadPriority");
  rb_define_const(mPrio, "Idle", INT2FIX(eIdle));
  rb_define_const(mPrio, "Lowest", INT2FIX(eLowest));
  rb_define_const(mPrio, "Low", INT2FIX(eLow));
  rb_define_const(mPrio, "Normal", INT2FIX(eNormal));
  rb_define_const(mPrio, "High", INT2FIX(eHigh));
  rb_define_const(mPrio, "Higher", INT2FIX(eHigher));
  rb_define_const(mPrio, "TimeCritical", INT2FIX(eTimeCritical));

  /************************************/
  /* define TunePimp::Callback module */
  /************************************/
  mCB = rb_define_module_under(mTP, "Callback");
  rb_define_const(mCB, "FileAdded", INT2FIX(tpFileAdded));
  rb_define_const(mCB, "FileChanged", INT2FIX(tpFileChanged));
  rb_define_const(mCB, "FileRemoved", INT2FIX(tpFileRemoved));
  rb_define_const(mCB, "WriteTagsComplete", INT2FIX(tpWriteTagsComplete));
  rb_define_const(mCB, "CallbackLast", INT2FIX(tpCallbackLast));

  /**********************************/
  /* define TunePimp::Status module */
  /**********************************/
  mStat = rb_define_module_under(mTP, "Status");
  rb_define_const(mStat, "Unrecognized", INT2FIX(eUnrecognized));
  rb_define_const(mStat, "Recognized", INT2FIX(eRecognized));
  rb_define_const(mStat, "Pending", INT2FIX(ePending));
  rb_define_const(mStat, "TRMLookup", INT2FIX(eTRMLookup));
  rb_define_const(mStat, "TRMCollision", INT2FIX(eTRMCollision));
  rb_define_const(mStat, "FileLookup", INT2FIX(eFileLookup));
  rb_define_const(mStat, "UserSelection", INT2FIX(eUserSelection));
  rb_define_const(mStat, "Verified", INT2FIX(eVerified));
  rb_define_const(mStat, "Deleted", INT2FIX(eDeleted));
  rb_define_const(mStat, "Error", INT2FIX(eError));

  /*********************************/
  /* define TunePimp::Error module */
  /*********************************/
  mErr = rb_define_module_under(mTP, "Error");
  rb_define_const(mErr, "Ok", INT2FIX(tpOk));
  rb_define_const(mErr, "TooManyTRMs", INT2FIX(tpTooManyTRMs));
  rb_define_const(mErr, "NoUserInfo", INT2FIX(tpNoUserInfo));
  rb_define_const(mErr, "LookupError", INT2FIX(tpLookupError));
  rb_define_const(mErr, "SubmitError", INT2FIX(tpSubmitError));
  rb_define_const(mErr, "InvalidIndex", INT2FIX(tpInvalidIndex));
  rb_define_const(mErr, "InvalidObject", INT2FIX(tpInvalidObject));
  rb_define_const(mErr, "ErrorLast", INT2FIX(tpErrorLast));

  /**************************************/
  /* define TunePimp::ResultType module */
  /**************************************/
  mRT = rb_define_module_under(mTP, "ResultType");
  rb_define_const(mRT, "None", INT2FIX(eNone));
  rb_define_const(mRT, "ArtistList", INT2FIX(eArtistList));
  rb_define_const(mRT, "AlbumList", INT2FIX(eAlbumList));
  rb_define_const(mRT, "TrackList", INT2FIX(eTrackList));
  rb_define_const(mRT, "MatchedTrack", INT2FIX(eMatchedTrack));
}
