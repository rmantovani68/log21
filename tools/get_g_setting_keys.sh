for i in `gsettings list-schemas`
    do 
        echo '-------------------- '$i
        # gsettings list-keys $i
        for j in `gsettings list-keys $i`
            do
                echo -en $j ' : '  
                gsettings get $i $j
            done
    done
