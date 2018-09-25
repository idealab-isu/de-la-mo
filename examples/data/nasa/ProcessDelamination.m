clear;
delamNames = {'delam12','delam23','delam34','delam56','delam67'};
numDelams = length(delamNames);
lengthX = 247;
lengthY = 101;

for delam=1:numDelams
	BW = load([delamNames{delam} '.txt']);
	[B,L,N,A] = bwboundaries(BW);
	figure;
	imagesc(BW);
	hold on;
    reverse = false;
	colors=['b' 'g' 'r' 'c' 'm' 'y'];
    switch delam
        case 1
            skip = 4;
        case 2
            skip = 5;
        case 3
            skip = 6;
        case 4
            skip = 4;
        case 5
            skip = 4;
        otherwise
            skip = 2;
    end
	for k=2:length(B),
		boundary = B{k};
		cidx = mod(k,length(colors))+1;
		plot([boundary(1:skip:end,2); boundary(end,2)], [boundary(1:skip:end,1); boundary(end,1)], colors(cidx),'LineWidth',2);
		fileName = [delamNames{delam} '-' num2str(k-1) '.csv'];
        if reverse
            delamOutlineX = [boundary(end:-skip:1,2); boundary(end,2)]./lengthX;
            delamOutlineY = 1.0-[boundary(end:-skip:1,1); boundary(end,1)]./lengthY;
        else
            delamOutlineX = [boundary(1:skip:end,2); boundary(end,2)]./lengthX;
            delamOutlineY = 1.0-[boundary(1:skip:end,1); boundary(end,1)]./lengthY;
        end
        delamOutlineZ = zeros(length(delamOutlineX),1);
		delamOutline = [delamOutlineX delamOutlineY delamOutlineZ];
		WriteCSVBoundaryFile(fileName, delamOutline);
	end
	axis equal;
	clear B L N A;
end