clc;
clear;
close;
filename = "PAdata_golden.txt";

cells = load(filename);
pinPos = load("PApinPos.txt");

[cellNum,~] = size(cells);
cellNum = cellNum-1;

figure();
Xmax = cells(end,2);
Xmin = cells(end,1);
Ymax = cells(end,3);
Ymin = cells(end,4);

width = Xmax - Xmin;
height = Ymax - Ymin;
rectangle('position',[Xmin-10000, Ymin-10000, width+20000, height+20000]);
axis equal;
axis off;
xlim([Xmin - 13000, Xmax + 13000]);
ylim([Ymin - 13000, Ymax + 13000]);
hold on; 

cells_rsp=cells;
cells_rsp(:,2)=cells(:,4);
cells_rsp(:,3)=cells(:,2)-cells(:,1);
cells_rsp(:,4)=cells(:,3)-cells(:,4);

centerX = (pinPos(:,1)+pinPos(:,2))/2;
centerY = (pinPos(:,3)+pinPos(:,4))/2;
center = [centerX centerY];
resX = mod(centerX - 170, 560);
resY = mod(centerY - 140, 560);

for ii=1:cellNum
    if cells_rsp(ii,5)==1
        rectangle('Position', cells_rsp(ii,1:4),'FaceColor','b');
    elseif cells_rsp(ii,5)==2
        rectangle('Position', cells_rsp(ii,1:4),'FaceColor','r','LineWidth',2.5);
    end
end
%end