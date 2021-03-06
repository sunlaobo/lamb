<?php

/*
 * The Message Model
 * Link http://github.com/typefo/lamb
 * Copyright (C) typefo <typefo@qq.com>
 */

use Tool\Filter;

class MessageModel {
    public $db = null;
    private $table = 'message';
    private $column = ['begin', 'end', 'spid', 'spcode', 'phone', 'content', 'status'];
    
    public function __construct() {
        $this->db = Yaf\Registry::get('db');
    }

    public function getAll() {
        $result = [];
        $sql = 'SELECT * FROM ' . $this->table . ' ORDER BY create_time DESC LIMIT 32';
        $sth = $this->db->query($sql);
        if ($sth) {
            $result = $sth->fetchAll();
        }

        return $result;
    }

    public function query(array $where) {
        $where = $this->checkArgs($where);

        if (isset($where)) {

        }
    }

    public function checkArgs(array $data) {
        $res = array();
        $data = array_intersect_key($data, array_flip($this->column));

        foreach ($data as $key => $val) {
            switch ($key) {
            case 'begin':
                $res['begin'] = Filter::date($val, date('Y-m-d H:i:s'));
            case 'spid':
                $res['spid'] = Filter::alpha($val, null, 1, 20);
                break;
            case 'spcode':
                $res['spcode'] = Filter::alpha($val, null, 1, 20);
                break;
            case 'phone':
                $res['phone'] = Filter::alpha($val, null, 1, 16);
                break;
            case 'content':
                $res['content'] = Filter::string($val, null, 1, 140);
                break;
            case 'status':
                $res['status'] = Filter::number($val, 1, 1, 7);
                break;
            }
        }

        return $res;
    }
}
