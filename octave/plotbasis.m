bold=true
dotted=true
##n = 6;
n=12;
for i=0:n;
  s = ['basis_',int2str(i),'.dat'];
  pseg=load(s);
  p = plot(pseg(:,1),pseg(:,2));
  set(p, "linewidth", 2)

  dotted_array = [];
  if dotted;
    if n==12;
      dotted_array = [1 2 4 5 7 8 10 11];
    else
      dotted_array = [1 2 4];
    endif
  end;

  Nd = size(dotted_array,2);
  for j=1:Nd;
    if i==dotted_array(j);
      set(p, "linestyle", '--')
    end;
  end;
  
  bold_array = [];
  if bold;
    if n==6;
      bold_array = [0 6];
    else;
      if n==12;
        bold_array = [0 12 3 6 9];
      endif
    endif
  endif
  
  Nb = size(bold_array,2);
  for j=1:Nb;
    if i==bold_array(j);
      set(p, "linewidth", 4)
    endif
  end;
  
  h=get(gcf, "currentaxes");
  set(h, "fontsize", 24, 'FontName', 'Times New Roman')
  hold on;
end;
print -dpng -r300 "bsbasis.png"
