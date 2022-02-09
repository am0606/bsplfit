n = 6;
for i=0:n;
  s = ['basis_',int2str(i),'.dat'];
  pseg=load(s);
  p = plot(pseg(:,1),pseg(:,2));
  set(p, "linewidth", 2)

  dotted_array = [];
  ##  dotted_array = [1 2 4];

  Nd = size(dotted_array,2);
  for j=1:Nd;
    if i==dotted_array(j);
      set(p, "linestyle", '--')
    end;
  end;
  
  bold_array = [];
  ##  bold_array = [0 6];
  Nb = size(bold_array,2);
  for j=1:Nb;
    if i==bold_array(j);
      set(p, "linewidth", 3)
    end;
  end;
  
  h=get(gcf, "currentaxes");
  set(h, "fontsize", 24, 'FontName', 'Times New Roman')
  hold on;
end;
print -dpng -r300 "bsbasis.png"
