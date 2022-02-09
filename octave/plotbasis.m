n = 6;
for i=0:n;
  s = ['basis_',int2str(i),'.dat'];
  pseg=load(s);
  p = plot(pseg(:,1),pseg(:,2));
  set(p, "linewidth", 2)  

  dotted_array = [];
  ##  dotted_array = [1 2 4];

  Na = size(dotted_array,2);
  for j=1:Na;    
    if i==j;
      set(p, "linestyle", '--')
    end;
  end;
  h=get(gcf, "currentaxes");
  set(h, "fontsize", 24, 'FontName', 'Times New Roman')
  hold on;
end;
print -dpng -r300 "bsbasis.png"
