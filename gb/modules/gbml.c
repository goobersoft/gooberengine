
/*
var/const
	ZMLNODE_LIST = 1
	ZMLNODE_SCOPE = 2
	ZMLNODE_S = 3

proc/ZMLNode(list/L, S=0)
//	set waitfor = 0
	var/z = new/list(3)
	z[ZMLNODE_SCOPE] = S
	z[ZMLNODE_LIST] = L
	z[ZMLNODE_S] = ""
	return z
*/

//////////////
// gbmlnode //
//////////////

#define gbmlnode_type_tree()   1
#define gbmlnode_type_int()    2
#define gbmlnode_type_string() 3

type() {

  // this indicates the current scope, 0 being top.
  string_t * key;
  int        scope;
  bool_t     data_type;
  void *     data;

} gbmlnode_t;


type() {

  tree_t * data;

} gbml_t;


/*

#define zml_tab() "  "

proc/zml_load(f)
	var/d = file2text(f+".zml")
	if(d != null)
		return zml_text_parse(d)
	else
		return new/list()

proc/zml_save(z,t)
	if(z == null) return

	if(fexists("[t]-TEMP.zml"))
		fdel("[t]-TEMP.zml")

	var/tm = file("[t]-TEMP.zml")
	try
		tm << zml_to_string(Z)

//		Temp = players/conta/admin/admin(1)-TEMP.zml
//		T = players/conta/admin/admin(1)
//		T-TEMP = players/conta/admin/admin(1)-TEMP
//		T-TEMP+.zml = players/conta/admin/admin(1)-TEMP.zml
//		T+zml = players/conta/admin/admin(1).zml

		if(length(Temp))
			fdel(T+".zml")
			fcopy(Temp,T+".zml")
			fdel(Temp)

	catch(var/exception/e)
		world.log << "[e] on [e.file]:[e.line]"
		fdel(Temp)


	fdel(T+".zml")
	var/f = file(T+".zml")
	f << ZML_ToString(Z)


proc/ZML_Text_Parse(T)
//	set waitfor = 0
	var/list/l = Tokenize(T, list("{","}","=",";","#"),list())
	var/list/r = list(".."=null)
	var/list/rc = r
	var/vn  = null
	var/vv = null
	var/s = 0
	var/cm = 0

	for (var/d in l)
		if(d == "{")
			if(!cm)
				if(vn != null)
					var/u = new/list()
					u[".."] = rc

					rc[vn] = u
					rc = rc[vn]

					vn = null
					vv = null

					s++
				else
					var/list/u = new
					u[".."] = rc
					rc["_"] = u
					rc = rc["_"]

					vn = null
					vv = null

					s++
			vn = null;
			vv = null;

		else if(d == "}")
			if(!cm)
				if(s > 0)
					var/uu = rc
					rc = rc[".."]
					uu -= ".."

					vn = null
					vv = null
					s--
				else
					CRASH("Scope drops below 0!")

		else if(d == "=")
			if(!cm)
				if(vn != null)
					rc[vn] = "0"
				else
					rc["_"] = "0"

		else if(d == ";")
			if(!cm)
				if( (vn != null) && (vv != null) )
					rc[vn] = vv
					vn = null
					vv = null
				else if(vn != null)
					rc += vn
					vn = null

		else if(d == "#")
			cm = !cm

		else
			if(!cm)
				if(vn == null)
					vn = d
				else if(vv == null)
					vv = d
	rc -= ".."
	return r

proc/ZML_ToString(Z)
//	set waitfor = 0
	return ZMLNode_ToString(ZMLNode(Z,0))

var/const
	ZMLNODE_LIST = 1
	ZMLNODE_SCOPE = 2
	ZMLNODE_S = 3

proc/ZMLNode(list/L, S=0)
//	set waitfor = 0
	var/z = new/list(3)
	z[ZMLNODE_SCOPE] = S
	z[ZMLNODE_LIST] = L
	z[ZMLNODE_S] = ""
	return z

proc/ZMLNode_ToString(Z)
//	set waitfor = 0
	if(Z[ZMLNODE_LIST])
		for (var/d in Z[ZMLNODE_LIST])
			d = string(d)
			if(islist(Z[ZMLNODE_LIST][d]))
				var/z = ZMLNode(Z[ZMLNODE_LIST][d], Z[ZMLNODE_SCOPE]+1)
				Z[ZMLNODE_S] += "[RepString(ZML_TAB,Z[ZMLNODE_SCOPE])][d] {\n" + ZMLNode_ToString(z)
				Z[ZMLNODE_S] += "[RepString(ZML_TAB,Z[ZMLNODE_SCOPE])]}\n"
			else
				if(Z[ZMLNODE_LIST][d] != null)
					Z[ZMLNODE_S] += "[RepString(ZML_TAB,Z[ZMLNODE_SCOPE])][d] = [Z[ZMLNODE_LIST][d]];\n"
				else
					Z[ZMLNODE_S] += "[RepString(ZML_TAB,Z[ZMLNODE_SCOPE])][d];\n"

	return Z[ZMLNODE_S]

///////////////////////
// GENERAL FUNCTIONS //
///////////////////////

#define gb_clamp(n,l,h) max(min(n,h),l)

proc/string(S)
	if      (isnum(S))   return num2text(S,100)
	else if (istext(S))  return S
	else                 return "[S]"

proc/RepString(T=" ", N=1)
	var/t = ""
	for (var/i = 1 to N)
		t += T
	return t

proc/Char(T,N)
	return ascii2text( text2ascii(T,N) )

proc/TrimString(T)
	var/p1 = 0
	var/p2 = 0
	var/list/l = list(" ","\t","\n")
	var/c  = ""
	for (var/i = 1 to length(T))
		c = Char(T,i)
		if( !(c in l))
			if(!p1)
				p1 = i
				p2 = i
			else
				p2 = i
	return copytext(T,p1,p2+1)

proc/Tokenize(T, list/L=list(","), list/M=list(";"))
	var/c = ""
	var/w = ""
	var/list/l = new
	for (var/i = 1 to length(T))
		c = Char(T,i)
		if (c in L)
			w = TrimString(w)
			if(w)
				l += w
			l += c
			w = ""
			c = ""
		else if (c in M)
			w = TrimString(w)
			if(w)
				l += w
			w = ""
			c = ""
		else
			w += c
	if(w)
		l += TrimString(w)
	return l
*/