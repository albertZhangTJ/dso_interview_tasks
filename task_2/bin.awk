BEGIN {
    bin1 = 0;  # [1,10)
    bin2 = 0;  # [10,100)
    bin3 = 0;  # [100,1000)
    bin4 = 0;  # [1000,inf)
}

{
    if ($1 >= 1 && $1 < 10) bin1++;
    else if ($1 >= 10 && $1 < 100) bin2++;
    else if ($1 >= 100 && $1 < 1000) bin3++;
    else if ($1 >= 1000) bin4++;
}

END {
    print "1 " bin1
    print "2 " bin2
    print "3 " bin3
    print "4 " bin4
}
