pseg=load("bspoints_seg.txt");
vseg=load("vertexes_seg.txt");
plot(pseg(:,1),pseg(:,2));
hold on
plot(vseg(:,1),vseg(:,2),'b');
print -dpng -r300 "bspl_curve_seg.png"