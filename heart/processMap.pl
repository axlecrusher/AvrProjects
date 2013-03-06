open(IN, "<portmap.txt");

while (<IN>)
{
	print ($_);
	process($_);
}

sub process
{
	my ($line) = @_;
	my ($type, $data) = split('=', $line);
	my @numbers = split(',', $data);

	my $r = "$type={";

	foreach my $n (@numbers)
	{
		my $value = 0;
		my @digits = ($n =~ /./g);
		foreach my $d (@digits)
		{
			$value += 1<<$d;
		}
		$r .= sprintf("0x%02X,",$value);
	}

	$r = substr($r,0,-1) . "}\n";
	print $r;
}
