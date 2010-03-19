my $n = 0;
my $tot_size = 0;
my $size;
my $size_max = 0;

while (<>) {
    chomp;
    ++ $n;

    $size = length($_) + 1;

    $size_max = $size if ($size > $size_max);

    $tot_size += $size;
}

my $avgsize = $tot_size / $n;

print "$n entries.\n";
print "$avgsize average length.\n";
print $avgsize / 260, " utilization.\n"; 
print "$size_max maximum size.\n";
