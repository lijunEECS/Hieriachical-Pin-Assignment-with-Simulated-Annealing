clc;
clear;
close;
filename = "PAdata.txt";

cells = load(filename);

[cellNum,~] = size(cells);
cellNum = cellNum-1;

figure();
Xmax = cells(end,2);
Xmin = cells(end,1);
Ymax = cells(end,3);
Ymin = cells(end,4);

width = Xmax - Xmin;
height = Ymax - Ymin;
rectangle('position',[Xmin, Ymin, width, height]);
axis off;
xlim([Xmin - 3000, Xmax + 3000]);
ylim([Ymin - 3000, Ymax + 3000]);
hold on; 

cells_rsp=cells;
cells_rsp(:,2)=cells(:,4);
cells_rsp(:,3)=cells(:,2)-cells(:,1);
cells_rsp(:,4)=cells(:,3)-cells(:,4);

for ii=1:cellNum
    if cells_rsp(ii,5)==1
        rectangle('Position', cells_rsp(ii,1:4),'FaceColor','b');
    elseif cells_rsp(ii,5)==2
        rectangle('Position', cells_rsp(ii,1:4),'FaceColor','r','LineWidth',5);
    end
end
%end