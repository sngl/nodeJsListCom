{
  'targets': [
    {
      "target_name": "listcom",
	  "sources": [ "jsListCom.cpp" ].
      "conditions": [
        ['OS=="win"',
          {
            "sources": [
              "src_win\listCom.c"]
            ],
          }
        ],
        ['OS!="win"',
          {
            "sources": [
              "src_mac/listCom.c"
              
            ],
          }
        ],
      ],
    },
  ],
}