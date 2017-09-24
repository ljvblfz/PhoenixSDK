{ program 11.5
 binary tree traversal }
 
program Traversal(traversal, output);

type ptr = ^node;
	node = record info : char;
				llink,rlink : ptr
		   end;
var root : ptr; ch : char; traversal : text;

procedure preorder(p : ptr);
begin if p<>nil then 
	  begin write(p^.info);
		preorder(p^.llink);
		preorder(p^.rlink)
	  end
end; {preorder}

procedure inorder(p : ptr);
begin if p<> nil then
	  begin inorder(p^.llink);
		write(p^.info);
		inorder(p^.rlink)		
	  end
end; {inorder}

procedure postorder(p : ptr);
begin if p<>nil then 
	  begin postorder(p^.llink);
		postorder(p^.rlink);
		write(p^.info)
	  end
end; {postorder}

procedure enter(var p:ptr);
begin read(traversal, ch); write(ch);
	if ch<>'.' then
	begin new(p);
		p^.info := ch;
		enter(p^.llink);
		enter(p^.rlink);
	end
	else p := nil
end; {enter}

procedure exit(p : ptr);
begin if p<>nil then 
	  begin exit(p^.llink);
		exit(p^.rlink);
		dispose(p)
	  end
end; {exit}

begin
	write(' '); enter(root); writeln;
	write(' '); preorder(root); writeln;
	write(' '); inorder(root); writeln;
	write(' '); postorder(root); writeln;
	exit(root)
end.