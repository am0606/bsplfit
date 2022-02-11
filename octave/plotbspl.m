spdisp=true
sddisp=true
p=load("bspoints.txt");
v=load("controlPoints.txt");
if spdisp;
  sp=load("selected_points.txt");
endif
if sddisp;
  sd=load("selected_ders.txt");
endif
sz_sd = size(sd,1);
sdn = zeros(sz_sd,3);
for i=1:sz_sd;
  sdn1 = sd(i,:)/norm(sd(i,:));
  sdn(i,:) = sdn1;
end;
plot(p)
hcurve=plot(p(:,1),p(:,2));
set(hcurve, "linewidth", 2)
hold on
hcp = plot(v(:,1),v(:,2),'go-');
set(hcp, "linewidth", 2)
if spdisp;
  plot(sp(:,1),sp(:,2),'r*');
endif
if sddisp;
  for i=1:sz_sd;
    quiver(sp(i,1),sp(i,2),sdn(i,1),sdn(i,2),'b');
  end;
endif
haxes=get(gcf, "currentaxes");
set(haxes, "fontsize", 24, 'FontName', 'Times New Roman')
print -dpng -r300 "bspl_curve.png"