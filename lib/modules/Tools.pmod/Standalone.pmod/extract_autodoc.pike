/*
 * $Id: extract_autodoc.pike,v 1.25 2002/12/10 13:45:36 grubba Exp $
 *
 * AutoDoc mk II extraction script.
 *
 * Henrik Grubbström 2001-01-08
 */

array(string) find_root(string path) {
  if(file_stat(path+"/.autodoc")) {
    // Note .autodoc files are space-separated to allow for modulenames like
    //      "7.0".
    return (Stdio.read_file(path+"/.autodoc")/"\n")[0]/" " - ({""});
  }
  if (!sizeof(path)) return ({});
  array(string) parts = path/"/";
  string name = parts[-1];
  sscanf(name, "%s.pmod", name);
  return find_root(parts[..sizeof(parts)-2]*"/") + ({ name });
}

string imgsrc;
string imgdir;

int main(int n, array(string) args) {

  if(n!=5) {
    write("%s <srcdir> <imgsrc> <builddir> <imgdir>\n", args[0]);
    exit(1);
  }

  string srcdir = combine_path(getcwd(), args[1]);
  imgsrc = combine_path(getcwd(), args[2]);
  string builddir = combine_path(getcwd(), args[3]);
  imgdir = combine_path(getcwd(), args[4]);
  if(srcdir[-1]!='/') srcdir += "/";
  if(builddir[-1]!='/') builddir += "/";
  if(imgdir[-1]!='/') imgdir += "/";
  recurse(srcdir, builddir);
}

void recurse(string srcdir, string builddir) {
  werror("Extracting from %s\n", srcdir);
  foreach(get_dir(srcdir), string fn) {
    if(fn=="CVS") continue;
    if(fn[0]=='.') continue;
    if(fn[-1]=='~') continue;
    if(fn[0]=='#' && fn[-1]=='#') continue;

    Stdio.Stat stat = file_stat(srcdir+fn, 1);

    if(stat->isdir) {
      if(!file_stat(builddir+fn)) mkdir(builddir+fn);
      recurse(srcdir+fn+"/", builddir+fn+"/");
      continue;
    }

    if(stat->size<1) continue;

    if(!has_suffix(fn, ".pike") && !has_suffix(fn, ".pike.in") &&
       !has_suffix(fn, ".pmod") && !has_suffix(fn, ".pmod.in") &&
       !has_suffix(fn, ".c")) continue;

    Stdio.Stat dstat = file_stat(builddir+fn+".xml");

    if(!dstat || dstat->mtime < stat->mtime) {
      string res = extract( srcdir+fn, imgdir, 0, builddir);
      if(!res) exit(1);
      Stdio.write_file(builddir+fn+".xml", res);
    }
  }
}

string extract(string filename, string imgdest, int(0..1) rootless, string builddir) {

  werror("Extracting file %O...\n", filename);
  string file = Stdio.read_file(filename);

  if (!file) {
    werror("WARNING: Failed to read file %O!\n", filename);
    return "\n";
  }

  int i;
  if (has_value(file, "**""!") ||
      (((i = search(file, "//! ""module ")) != -1) &&
       (sizeof(array_sscanf(file[i+11..],"%s\n%*s")[0]/" ") == 1))) {
    // Mirar-style markup.
    Tools.AutoDoc.MirarDocParser mirar_parser =
      Tools.AutoDoc.MirarDocParser(imgsrc);
    int lineno = 1;
    foreach(file/"\n", string line) {
      mirar_parser->process_line(line, filename, lineno++);
    }
    return mirar_parser->make_doc_files( builddir, imgdest );
  }

  string name_sans_suffix, suffix;
  if (has_suffix(filename, ".in")) {
    name_sans_suffix = filename[..sizeof(filename)-4];
  }
  else
    name_sans_suffix = filename;
  if(!has_value(name_sans_suffix, "."))
    error("No suffix in file %O.\n", name_sans_suffix);
  suffix = ((name_sans_suffix/"/")[-1]/".")[-1];
  if( !(< "c", "pike", "pmod", >)[suffix] )
    error("Unknown filetype %O.\n", suffix);
  name_sans_suffix = name_sans_suffix[..sizeof(name_sans_suffix)-(sizeof(suffix)+2)];

  string result;
  mixed err = catch {
    if( suffix == "c" )
      result = Tools.AutoDoc.ProcessXML.extractXML(filename);
    else {
      array(string) parents = rootless?({}):find_root(dirname(filename));
      string type = ([ "pike":"class", "pmod":"module", ])[suffix];
      string name = (name_sans_suffix/"/")[-1];
#if 0
      werror("parents: %{%O, %}\n"
	     "type: %O\n"
	     "name: %O\n", parents, type, name);
#endif /* 0 */
      if(name == "master.pike")
	name = "/master";
      if(name == "module" && (filename/"/")[-1] != "module.pike" && !rootless) {
	if(!sizeof(parents))
	  error("Unknown module parent name.\n");
	name = parents[-1];
	parents = parents[..sizeof(parents)-2];
      }

      result = Tools.AutoDoc.ProcessXML.extractXML(filename, 1, type, name, parents);
    }
  };

  if (err) {
    if (arrayp(err) && _typeof(err[0]) <= Tools.AutoDoc.AutoDocError)
      werror("%O\n", err[0]);
    else if (objectp(err) && _typeof(err) <= Tools.AutoDoc.AutoDocError)
      werror("%O\n", err);
    else
      werror("%s\n", describe_backtrace(err));

    return 0;
  }

  if(result && sizeof(result))
    return Tools.AutoDoc.ProcessXML.moveImages(result, builddir, imgdest);

  return "\n";
}
